#include <Arduino.h>
#include "utils.h"

// PWM variables:
// Pin number.
volatile uint8_t pwmPin = 6;
// High value for PWM.
volatile uint8_t pwmHigh = 180;
// Low value for PWM.
volatile uint8_t pwmLow = 0;

// Backlight controller.
void backlight(uint8_t mode)
{
    if (mode == ON)
    {
        analogWrite(pwmPin, pwmHigh);
    }
    else if (mode == OFF)
    {
        analogWrite(pwmPin, pwmLow);
    }
}

void printTimestamp()
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
}

// Serial port value logger.
void log(int value)
{
    printTimestamp();
    Serial.println(value);
    Serial.flush();
}

// Serial port logger.
void log(const char *msg)
{
    printTimestamp();
    Serial.println(msg);
    Serial.flush();
}
