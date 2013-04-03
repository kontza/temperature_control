#include <avr/sleep.h>
#include <avr/io.h>
#define NO_PIN_NUMBER       // to indicate that you don't need the arduinoPin
#include <PinChangeInt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <leOS2.h>
leOS2 scheduler;
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 11, 5, 4, 3);

// Declare and initialize the semaphore.
uint8_t isrSem = 0;
uint8_t idleSem = 0;
volatile unsigned long idleTimeout = 0;
volatile uint8_t pwmPin = 6, pwmHigh = 180, pwmLow = 0;

void log(const char *msg)
{
    unsigned long timestamp = millis();
    unsigned long workvalue = timestamp;
    unsigned long result;
    unsigned long divisor = 100000;
    while (divisor)
    {
        result = workvalue / divisor;
        workvalue = workvalue - result * divisor;
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
    cli();
    PCdetachInterrupt(7);
    sei();
    scheduler.restartTask(keyHandler);
}

void keyHandler()
{
    long keyValue = 0, tmpValue = 11, keyCount = 0;
    static const unsigned long initialKeyRepeatTimer = 300;
    unsigned long timeOut, keyRepeatTimer = initialKeyRepeatTimer;
    log("KT: resumed.");
    idleTimeout = millis() + 5000;
    scheduler.pauseTask(keyHandler);
}

void idle()
{
    if (millis() < idleTimeout)
    {
        return;
    }
    log("ID: time to sleep");
    analogWrite(pwmPin, pwmLow);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
    sei();
    PCattachInterrupt(7, keyIsr, RISING);
    // Disable scheduler by disabling WDT ISR.
    scheduler.haltScheduler();
    sleep_cpu();
    // Restart scheduler.
    scheduler.restartScheduler();
    sleep_disable();
    analogWrite(pwmPin, pwmHigh);
    log("ID: wake up");
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
    scheduler.begin(10);
    scheduler.addTask(keyHandler, scheduler.convertMs(100));
    scheduler.addTask(idle, scheduler.convertMs(100));
    // Put both tasks on pause.
    idleTimeout = millis() + 5000;
    scheduler.pauseTask(keyHandler);
}

void loop()
{
}
