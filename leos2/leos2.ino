#include <avr/sleep.h>
#include <avr/io.h>
#define NO_PIN_NUMBER       // to indicate that you don't need the arduinoPin
#include <PinChangeInt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <leOS2.h>
#include "utils.h"
leOS2 scheduler;

// Adafruit_PCD8544(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS, int8_t RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 5, 4, 3);

// Countdown value until power down.
volatile unsigned long idleTimeout = 0;

// PWM variables:
// Pin number.
volatile uint8_t pwmPin = 6, pwmHigh = 180, pwmLow = 0;
// High value for PWM.
volatile uint8_t pwmHigh = 180;
// Low value for PWM.
volatile uint8_t pwmLow = 0;

// Keyboard ISR releases keyHandler from pause state.
void keyIsr()
{
    cli();
    PCdetachInterrupt(7);
    sei();
    scheduler.restartTask(keyHandler);
}

// Handle key input.
void keyHandler()
{
    long keyValue = 0, tmpValue = 11, keyCount = 0;
    static const unsigned long initialKeyRepeatTimer = 300;
    unsigned long timeOut, keyRepeatTimer = initialKeyRepeatTimer;
    log("KT: resumed.");
    idleTimeout = millis() + 5000;
    scheduler.pauseTask(keyHandler);
    PCattachInterrupt(7, keyIsr, RISING);
}

// Idle task waits until timeout occurs and puts the CPU to sleep.
void idle()
{
    long timeToStayUp = (long)(idleTimeout-millis());
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Count-down:");
    display.println(timeToStayUp);
    display.display();
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
    // Disable scheduler so that WDT ISR won't wake us up.
    scheduler.haltScheduler();
    digitalWrite(3, LOW);
    sleep_cpu();
    sleep_disable();
    analogWrite(pwmPin, pwmHigh);
    digitalWrite(3, HIGH);
    initDisplay();
    log("ID: wake up");
    // Restart scheduler.
    scheduler.restartScheduler();
}

// Initialize the display.
void initDisplay()
{
    display.begin();
    display.setContrast(50);
    display.setTextColor(BLACK);
}

// Stock function.
void setup()
{
    Serial.begin(9600);
    analogWrite(pwmPin, pwmHigh);
    pinMode(7, INPUT);
    digitalWrite(7, LOW);
    initDisplay();
    scheduler.begin(10);
    scheduler.addTask(keyHandler, scheduler.convertMs(100));
    scheduler.addTask(idle, scheduler.convertMs(500));
    // Put key task on pause.
    idleTimeout = millis() + 5000;
    scheduler.pauseTask(keyHandler);
}

// With leOS2, we don't need the loop-routine.
void loop()
{
}
