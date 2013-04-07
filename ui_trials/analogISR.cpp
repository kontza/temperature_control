/* leOS2 Interrupt Based Analog Read Library
 * BASED ON:
 * "Arduino NilRTOS Library
 * Copyright (C) 2013 by William Greiman"
 */
#include <leOS2.h>
#include <Arduino.h>

extern leOS2 scheduler;
static uint8_t _analog_reference = DEFAULT;
void keyHandler();

/** ADC ISR. */
ISR(ADC_vect)
{
    scheduler.restartTask(keyHandler);
}

//------------------------------------------------------------------------------
/**
 * Configures the reference voltage used for analog input
 * (i.e. the value used as the top of the input range). The options are:
 * @param[in] mode the ADC reference mode.
    - DEFAULT: the default analog reference of 5 volts (on 5V Arduino boards)
      or 3.3 volts (on 3.3V Arduino boards)
    - INTERNAL: an built-in reference, equal to 1.1 volts on the ATmega168 or
      ATmega328 and 2.56 volts on the ATmega8 (not available on the Arduino Mega)
    - INTERNAL1V1: a built-in 1.1V reference (Arduino Mega only)
    - INTERNAL2V56: a built-in 2.56V reference (Arduino Mega only)
    - EXTERNAL: the voltage applied to the AREF pin (0 to 5V only) is used
      as the reference.
*/
void leOS2AnalogReference(uint8_t mode)
{
    // can't actually set the register here because the default setting
    // will connect AVCC and the AREF pin, which would cause a short if
    // there's something connected to AREF.
    _analog_reference = mode;
}

//! Read an analog value from the given pin. Puts keyHandler on pause while waiting for the IRQ.
int leOS2AnalogRead(uint8_t pin)
{
#if defined(__AVR_ATmega32U4__)
    pin = analogPinToChannel(pin);
    ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#elif defined(ADCSRB) && defined(MUX5)
    // the MUX5 bit of ADCSRB selects whether we're reading from channels
    // 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
    ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif

    // set the analog reference (high two bits of ADMUX) and select the
    // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
    // to 0 (the default).
#if defined(ADMUX)
    ADMUX = (_analog_reference << 6) | (pin & 0x07);
#endif

    ADCSRA |= (1 << ADIE) | (1 << ADSC);
    scheduler.pauseTask(keyHandler);
    ADCSRA &= ~(1 << ADIE);

    // this will access ADCL first.
    return ADC;
}
