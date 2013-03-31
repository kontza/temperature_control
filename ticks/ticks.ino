#include <leOS2.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

leOS2 scheduler;

/*
    pin 7 - Serial clock out (SCLK)
    pin 6 - Serial data out (DIN)
    pin 5 - Data/Command select (D/C)
    pin 4 - LCD chip select (CS)
    pin 3 - LCD reset (RST)
*/
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 11, 5, 4, 3);

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Backlight control data.
int backlightPin = 9, lowPwm, highPwm = 182, pwmStep = 10, pwmValue;

// Temperature sensor count.
uint8_t temperatureSensorCount;
DeviceAddress deviceAddress;

void setup()
{
    scheduler.begin();
    Serial.begin(9600);

    // Start up the library
    sensors.begin();
    if (!sensors.getAddress(deviceAddress, 0))
    {
        memset(&deviceAddress, 0, sizeof(DeviceAddress));
    }
    else
    {
        sensors.requestTemperatures();
    }

    display.begin();
    // you can change the contrast around to adapt the display
    // for the best viewing!
    display.clearDisplay();   // clears the screen and buffer
    display.setContrast(50);
    display.display();

    // Tweak backlight.
    analogWrite(backlightPin, highPwm);
    scheduler.addTask(ticker, scheduler.convertMs(1212));
}

static int counter;
void ticker()
{
    counter++;
}

void loop()
{
    // Print temperature.
    display.clearDisplay();
    display.setTextColor(BLACK);
    display.setCursor(0, 8);
    display.print("Tick: ");
    display.print(counter);
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.setCursor(36, 0);
    if (!deviceAddress)
    {
        display.print("NO SENSOR");
    }
    else
    {
        float currentTemperature = 0.0;
        if (sensors.isConversionAvailable(deviceAddress))
        {
            currentTemperature = sensors.getTempC(deviceAddress);
            sensors.requestTemperatures();
        }
        else
        {
            currentTemperature = DEVICE_DISCONNECTED - 1;
        }
        if (currentTemperature == DEVICE_DISCONNECTED)
        {
            display.print("DISCONN");
        }
        else if (currentTemperature == DEVICE_DISCONNECTED - 1)
        {
            display.print("WAIT");
        }
        else
        {
            display.print(currentTemperature);
        }
    }

    // Done for now.
    display.display();
    delay(5000);
}
