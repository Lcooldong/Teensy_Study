#include "Indicator.h"

Indicator::Indicator()
{

}

Indicator::~Indicator()
{
}


void Indicator::breathe(uint8_t _delay)
{
  if(millis() - breathingTime > _delay)
  {
    breathingTime = millis();
    if(breathingDirection == true)
    {
      breathingValue++;
    }
    else
    {
      breathingValue--;
    }

    if(breathingValue >= 255)
    {
      breathingDirection = false;
    }
    else if(breathingValue <= 0) 
    {
      breathingDirection = true;
    }

    analogWrite(LED_BUILTIN, breathingValue);

  }
}