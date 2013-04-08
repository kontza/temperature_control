#include <avr/sleep.h>
#include <avr/io.h>
#define NO_PIN_NUMBER       // to indicate that you don't need the arduinoPin
#include <PinChangeInt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <leOS2.h>
#include "analogISR.h"
#include "utils.h"
leOS2 scheduler;

// Idle "ticks" until sleep.
#define INITIAL_ACTIVITY_TIMER 10

// Pin for keyboard pin change interrupt.
#define KEYBOARD_PIN 7

// Adafruit_PCD8544(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS, int8_t RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 5, 4, 3);

// Countdown value until power down.
volatile uint8_t activityTimer;

// Keyboard ISR releases keyHandler from pause state.
void keyIsr()
{
    cli();
    PCdetachInterrupt(KEYBOARD_PIN);
    sei();
    scheduler.restartTask(keyHandler);
}

// Handle key input.
void keyHandler()
{
    static const unsigned long initialKeyRepeatTimer = 300;
    unsigned long timeOut, keyRepeatTimer = initialKeyRepeatTimer;
    int tmpValue = 11, keyCount = 0, keyValue;
    log("KT: resumed.");
    keyValue = leOS2AnalogRead(0);
    log("KT: ADC done.");
    waitForKey();
}

// Wait for a key.
void waitForKey()
{
    activityTimer = INITIAL_ACTIVITY_TIMER;
    scheduler.pauseTask(keyHandler);
    PCattachInterrupt(KEYBOARD_PIN, keyIsr, RISING);
}

// Idle task waits until timeout occurs and puts the CPU to sleep.
static uint8_t firstIdle = 1;
void idle()
{
    if (firstIdle)
    {
        firstIdle = 0;
        display.clearDisplay();
    }
    else
    {
        display.fillRect(0, 0, 84, 16, 0);
    }
    display.setCursor(0, 0);
    display.println("Count-down:");
    display.println(--activityTimer);
    unsigned long alfa = millis();
    display.display();
    // Serial log the time taken by display.display().
    log((int)(millis() - alfa));
    if (activityTimer > 0)
    {
        return;
    }
    log("ID: time to sleep");
    backlight(OFF);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
    sei();
    PCattachInterrupt(KEYBOARD_PIN, keyIsr, RISING);
    // Disable scheduler so that WDT ISR won't wake us up.
    scheduler.haltScheduler();
    digitalWrite(3, LOW);
    sleep_cpu();
    sleep_disable();
    digitalWrite(3, HIGH);
    initDisplay();
    log("ID: wake up");
    // Restart scheduler.
    scheduler.restartScheduler();
}

// Initialize the display.
void initDisplay()
{
    backlight(ON);
    display.begin();
    display.setContrast(50);
    display.setTextColor(BLACK);
}

// Stock function.
void setup()
{
    Serial.begin(9600);
    pinMode(KEYBOARD_PIN, INPUT);
    digitalWrite(KEYBOARD_PIN, LOW);
    initDisplay();
    scheduler.begin(10);
    scheduler.addTask(keyHandler, scheduler.convertMs(100));
    scheduler.addTask(idle, scheduler.convertMs(500));
    // Put key task on pause.
    waitForKey();
}

// With leOS2, we don't need the loop-routine.
void loop()
{
}
