/*
Prints the given string out to the arduino serial port.
Message is prefixed with a millisecond timestamp.
*/
void log(const char *msg);

/*
Prints the given value out to the arduino serial port.
Message is prefixed with a millisecond timestamp.
*/
void log(int value);

/*
Controls the backlight.
*/
#define OFF 0
#define ON 1
void backlight(uint8_t mode);
