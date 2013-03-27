#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <analogComp.h>

// Set up the display.
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 11, 5, 4, 3);

// Backlight control data.
int backlightPin = 9, lowPwm, highPwm = 182, pwmStep = 10, pwmValue;

// Modes: RISING, FALLING, or CHANGE.
uint8_t comparatorMode = FALLING;

void setup()
{
    Serial.begin(9600);

    // you can change the contrast around to adapt the display
    // for the best viewing!
    display.begin();
    display.clearDisplay();   // clears the screen and buffer
    display.setContrast(50);
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.display();

    // Tweak backlight.
    analogWrite(backlightPin, highPwm);

    // Set up analog comparator.
    analogComparator.setOn(INTERNAL_REFERENCE, AIN1);
    analogComparator.enableInterrupt(handleFalling, CHANGE);
}

char *up = "O";
void handleFalling()
{
    up[0] = 'X';
    Serial.println("CHANGE");
    // analogComparator.enableInterrupt(handleRising, CHANGE);
}

void handleRising()
{
    up[0] = 'X';
    Serial.println("UP");
    analogComparator.enableInterrupt(handleFalling, FALLING);
}

void loop()
{
    // Print temperature.
    display.clearDisplay();
    display.setCursor(36, 0);
    display.print(up);
    display.setCursor(18, 16);
    display.print("O");
    display.setCursor(54, 16);
    display.print("O");
    // display.setCursor(36, 32);
    display.setCursor(0, 32);
    display.print("A0:");
    display.setCursor(36, 32);
    display.print(analogRead(A0));

    // Done for now.
    display.display();

    delay(1000);
}
