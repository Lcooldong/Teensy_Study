#ifndef __INDICATOR_H__
#define __INDICATOR_H__

#include <Arduino.h>

class Indicator
{
private:

    unsigned long long breathingTime = 0;
    unsigned char breathingValue = 0;
    bool breathingDirection = true;

public:
    Indicator();
    ~Indicator();

    void breathe(uint8_t _delay);

};



#endif