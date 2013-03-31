#include <avr/sleep.h>
#include <avr/io.h>
#define NO_PIN_NUMBER       // to indicate that you don't need the arduinoPin
#include <PinChangeInt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <pt.h>
#include <pt-sem.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 11, 5, 4, 3);

// Declare and initialize the semaphore.
static volatile struct pt_sem isrSem;
static struct pt_sem idleSem;
static struct pt pt_KeyHandler, pt_Idle;
static int idleTimer = 0;
volatile uint8_t pwmPin = 6, pwmHigh = 180, pwmLow = 0;

void log(const char *msg)
{
    unsigned long timestamp = millis();
    unsigned long workvalue = timestamp;
    unsigned long result;
    unsigned long divisor = 100000;
    while (divisor)
    {
        // Serial.print("DIV:");
        // Serial.println(divisor);
        // Serial.flush();
        result = workvalue / divisor;
        // Serial.print("RES:");
        // Serial.println(result);
        // Serial.flush();
        // Serial.print("WV 1:");
        // Serial.println(workvalue);
        // Serial.flush();
        workvalue = workvalue - result * divisor;
        // Serial.print("WV 2:");
        // Serial.println(workvalue);
        // Serial.flush();
        if (!result)
        {
            Serial.print(' ');
        }
        else
        {
            break;
        }
        divisor /= 10;
    }
    Serial.print(timestamp);
    Serial.print(' ');
    Serial.println(msg);
    Serial.flush();
}

void keyIsr()
{
    /* On AVR this forces compiler to save registers r18-r31.*/
    PCdetachInterrupt(7);

    /* Signal handler thread. */
    PT_SEM_SIGNAL(NULL, &isrSem);
}

PT_THREAD(keyHandler(struct pt *pt))
{
    int keyValue = 0, tmpValue = 20, timer;

    PT_BEGIN(pt);
    while (1)
    {
        // wait for event
        log("KT: wait sem");
        idleTimer = millis();
        do
        {
            PT_WAIT_UNTIL(pt, millis() > idleTimer + 5000 || isrSem.count > 0);
            if (isrSem.count > 0)
            {
                log("KT: sem seen");
                --isrSem.count;
            }
            else
            {
                log("KT: timeout, yield for idle");
                PT_SEM_SIGNAL(pt, &idleSem);
                PT_YIELD(pt);
            }
        }
        while (0);
        while (tmpValue > 10)
        {
            log("KT: repeat wait");
            idleTimer = millis();
            // Wait for 500ms.
            timer = idleTimer + 500;
            PT_WAIT_UNTIL(pt, millis() > timer);
            // Get key value.
            tmpValue = analogRead(0);
            keyValue = tmpValue;
            log("KT: wait over & processed");
        }
        log("KT: loop over.");
        tmpValue = 20;
        if (millis() > idleTimer + 5000)
        {
            log("KT: idle sem signalled");
            PT_SEM_SIGNAL(pt, &idleSem);
        }
        else
        {
            log("KT: wait for keyIsr");
            PCattachInterrupt(7, keyIsr, RISING);
        }
    }
    PT_END(pt);
}

void setup()
{
    Serial.begin(9600);
    analogWrite(pwmPin, pwmHigh);
    display.begin();
    display.setContrast(50);
    display.setTextColor(BLACK);
    display.display();
    pinMode(7, INPUT);
    digitalWrite(7, LOW);
    PT_SEM_INIT(&isrSem, 0);
    PT_SEM_INIT(&idleSem, 0);
    PT_INIT(&pt_KeyHandler);
    PT_INIT(&pt_Idle);
}

PT_THREAD(idle(struct pt *pt))
{
    PT_BEGIN(pt);
    while (1)
    {
        log("ID: wait start");
        PT_SEM_WAIT(pt, &idleSem);
        log("ID: sem seen, sleep");
        analogWrite(pwmPin, pwmLow);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        cli();
        sleep_enable();
        sei();
        PCattachInterrupt(7, keyIsr, RISING);
        sleep_cpu();
        sleep_disable();
        analogWrite(pwmPin, pwmHigh);
        log("ID: wake up");
    }
    PT_END(pt);
}

//------------------------------------------------------------------------------
// Loop is the idle thread.  The idle thread must not invoke any
// kernel primitive able to change its state to not runnable.
void loop()
{
    idle(&pt_Idle);
    keyHandler(&pt_KeyHandler);
}
