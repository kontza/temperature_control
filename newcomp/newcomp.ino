#define AC_REGISTER ADCSRB
//check if the micro is supported
#if defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328P__)
#define ATMEGAx8
#define NUM_ANALOG_INPUTS 6
#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)
#define ATTINYx5
#define NUM_ANALOG_INPUTS 4
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega8A__)
#define ATMEGA8
#undef AC_REGISTER
#define AC_REGISTER SFIOR
#define NUM_ANALOG_INPUTS 6
#elif defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
#define ATTINYx4
#define NUM_ANALOG_INPUTS 8
#elif defined (__AVR_ATmega640__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1281__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
#define ATMEGAx0
#define NUM_ANALOG_INPUTS 16
#elif defined (__AVR_ATmega344__) || defined (__AVR_ATmega344P__) || defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644PA__) || defined (__AVR_ATmega1284P__)
#define ATMEGAx4
#define NUM_ANALOG_INPUTS 8
#elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny4313__)
#define ATTINYx313
#define NUM_ANALOG_INPUTS 0
#elif defined (__AVR_ATmega32U4__)
#define ATMEGAxU
#define NUM_ANALOG_INPUTS 12
#else
#error Sorry, microcontroller not supported!
#endif

const uint8_t AIN0 = 0;
const uint8_t INTERNAL_REFERENCE = 1;
const uint8_t AIN1 = 255;
uint8_t oldADCSRA;

//setting and switching on the analog comparator
uint8_t setOn(uint8_t tempAIN0, uint8_t tempAIN1)
{
    //initialize the analog comparator (AC)
    ACSR &= ~(1 << ACIE); //disable interrupts on AC
    ACSR &= ~(1 << ACD); //switch on the AC

    //choose the input for non-inverting input
    if (tempAIN0 == INTERNAL_REFERENCE)
    {
        ACSR |= (1 << ACBG); //set Internal Voltage Reference (1V1)
    }
    else
    {
        ACSR &= ~(1 << ACBG); //set pin AIN0
    }

    //for Atmega32U4, only ADMUX is allowed as input for AIN-
#ifdef ATMEGAxU
    if (tempAIN1 == AIN1)
    {
        tempAIN1 = 0; //choose ADC0
    }
#endif

    //AtTiny2313/4313 don't have ADC, so inputs are always AIN0 and AIN1
#ifndef ATTINYx313
    //choose the input for inverting input
    if ((tempAIN1 >= 0) && (tempAIN1 < NUM_ANALOG_INPUTS))   //set the AC Multiplexed Input using an analog input pin
    {
        oldADCSRA = ADCSRA;
        ADCSRA &= ~(1 << ADEN);
        ADMUX &= ~31; //reset the first 5 bits
        ADMUX |= tempAIN1; //choose the ADC channel (0..NUM_ANALOG_INPUTS-1)
        AC_REGISTER |= (1 << ACME);
    }
    else
    {
        AC_REGISTER &= ~(1 << ACME); //set pin AIN1
    }
#endif

    //disable digital buffer on pins AIN0 && AIN1 to reduce current consumption
#if defined(ATTINYx5)
    DIDR0 &= ~((1 << AIN1D) | (1 << AIN0D));
#elif defined(ATTINYx4)
    DIDR0 &= ~((1 << ADC2D) | (1 << ADC1D));
#elif defined (ATMEGAx4)
    DIDR1 &= ~(1 << AIN0D);
#elif defined (ATTINYx313)
    DIDR &= ~((1 << AIN1D) | (1 << AIN0D));
#elif defined (ATMEGAx8) || defined(ATMEGAx4) || defined(ATMEGAx0)
    DIDR1 &= ~((1 << AIN1D) | (1 << AIN0D));
#endif
    return 0; //OK
}

void enableInterrupt(uint8_t tempMode)
{
    // Disable interrupts.
    SREG &= ~(1 << SREG_I);
    ACSR &= ~(1 << ACIE);

    //set the interrupt mode
    if (tempMode == CHANGE)
    {
        ACSR &= ~((1 << ACIS1) | (1 << ACIS0)); //interrupt on toggle event
    }
    else if (tempMode == FALLING)
    {
        ACSR &= ~(1 << ACIS0);
        ACSR |= (1 << ACIS1);
    }
    else     //default is RISING
    {
        ACSR |= ((1 << ACIS1) | (1 << ACIS0));
    }

    // Enable interrupts.
    ACSR |= (1 << ACIE);
    SREG |= (1 << SREG_I);
}

volatile uint16_t counter = 0;
ISR(ANALOG_COMP_vect)
{
    counter++;
}

void setup()
{
    Serial.begin(9600);
    setOn(INTERNAL_REFERENCE, AIN1);
    enableInterrupt(CHANGE);
}

void loop()
{
    Serial.print("Wait...");
    static int target = millis() + 1000;
    while (millis() < target);
    Serial.print("Counter... ");
    Serial.println(counter);
}
