/* leOS2 Interrupt Based Analog Read Library
 * BASED ON:
 * "Arduino NilRTOS Library
 * Copyright (C) 2013 by William Greiman"
 */
#ifndef ANALOG_ISR_H
#define ANALOG_ISR_H

int leOS2AnalogRead(uint8_t pin);
void leOS2AnalogReference(uint8_t mode);

#endif  // ANALOG_ISR_H
