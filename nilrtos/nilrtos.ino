/* Example of a handler thread triggered from an ISR by using a semaphore.
 * The handler message with response time should occur between the
 * "High" and "Low" messages from thread 2.
 */
#include <avr/sleep.h>
#include <avr/io.h>
#include <NilRTOS.h>
#include <NilAnalog.h>
#include <NilTimer1.h>
#define NO_PIN_NUMBER       // to indicate that you don't need the arduinoPin
#include <PinChangeInt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 11, 5, 4, 3);

// Uncomment the next two lines to save RAM and flash
// with the smaller unbuffered NilSerial library.
//#include <NilSerial.h>
//#define Serial NilSerial
volatile unsigned int keySeen = 0;

// Declare and initialize the semaphore.
SEMAPHORE_DECL(isrSem, 0);

//------------------------------------------------------------------------------
/* Fake ISR, normally void isrFcn()
 * would be replaced by something like
 * NIL_IRQ_HANDLER(INT0_vect).
 */
// NIL_IRQ_HANDLER(ANALOG_COMP_vect)
void keyIsr()
{
    /* On AVR this forces compiler to save registers r18-r31.*/
    NIL_IRQ_PROLOGUE();
    PCdetachInterrupt(7);

    /* IRQ handling code, preemptable if the architecture supports it.*/

    /* Nop on AVR.*/
    nilSysLockFromIsr();

    /* Invocation of some I-Class system APIs, never preemptable.*/

    /* Signal handler thread. */
    nilSemSignalI(&isrSem);

    /* Nop on AVR.*/
    nilSysUnlockFromIsr();

    /* More IRQ handling code, again preemptable.*/

    /* Epilogue performs rescheduling if required.*/
    NIL_IRQ_EPILOGUE();
}
//------------------------------------------------------------------------------
// Handler thread for interrupt.

// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);

// Declare thread function for thread 1.
NIL_THREAD(Thread1, arg)
{
    int keyValue = 0, tmpValue = 20;

    Serial.println("KT: start.");
    Serial.flush();
    while (1)
    {
        // wait for event
        nilSemWait(&isrSem);
        Serial.println("KT: sem seen.");
        Serial.flush();
        // Wait for 100ms.
        nilTimer1Start(500000);
        while (tmpValue > 10)
        {
            nilTimer1Wait();
            Serial.println("KT: timer done.");
            Serial.flush();
            // do ADC
            tmpValue = nilAnalogRead(0);
            keyValue = tmpValue;
            Serial.print("Key thread:");
            Serial.println(keyValue);
            Serial.flush();
        }
        tmpValue = 20;
        keySeen = millis();
        nilTimer1Stop();
    }
}

//------------------------------------------------------------------------------
/*
 * Threads static table, one entry per thread.  A thread's priority is
 * determined by its position in the table with highest priority first.
 *
 * These threads start with a null argument.  A thread's name may also
 * be null to save RAM since the name is currently not used.
 */
NIL_THREADS_TABLE_BEGIN()
NIL_THREADS_TABLE_ENTRY("Thread1", Thread1, NULL, waThread1, sizeof(waThread1))
NIL_THREADS_TABLE_END()
//------------------------------------------------------------------------------
volatile uint8_t pwmPin = 6, pwmHigh = 180;

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
    // Start NilRTOS.
    nilSysBegin();
}

void go_to_sleep()
{
    Serial.println("Sleep.");
    Serial.flush();
    set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
    sei();
    PCattachInterrupt(7, keyIsr, RISING);
    sleep_cpu();
    sleep_disable();
    analogWrite(pwmPin, pwmHigh);
    Serial.println("Wake up.");
    Serial.flush();
}

//------------------------------------------------------------------------------
// Loop is the idle thread.  The idle thread must not invoke any
// kernel primitive able to change its state to not runnable.
void loop()
{
    static int counter = 0;
    counter++;
    Serial.print("Counter: ");
    Serial.println(counter);
    Serial.flush();
    if (keySeen + 500 < millis())
    {
        go_to_sleep();
    }
}
