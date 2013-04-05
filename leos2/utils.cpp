// Serial port logger.
void log(const char *msg)
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
    Serial.println(msg);
    Serial.flush();
}
