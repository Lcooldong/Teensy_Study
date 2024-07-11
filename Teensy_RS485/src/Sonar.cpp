#include "Sonar.h"
#include <Arduino.h>

//#define DEBUG

Sonar::Sonar(SoftwareSerial* _serial)
{
    mySWSerial = _serial;
}

Sonar::Sonar(HardwareSerial* _serial)
{
    myHWSerial = _serial;
}

Sonar::~Sonar()
{
}


void Sonar::initSonar()
{
    myHWSerial->begin(9600);
}


int Sonar::getDistance()
{
  
  if(millis() - sonarLastTime > UART_Interval)
  {
    sonarLastTime = millis();

    receivedData = myHWSerial->read();
    // Serial.printf("SINGLE : 0x%X \r\n", receivedData);
    if(receivedData == 0xFF)
    {
        data[0] = 0xFF;
        receievdCount = 0;
    }
    else
    {       
        receievdCount++;     
        data[receievdCount] = receivedData; 

    //   Serial.printf("READ %02X %02X %02X %02X => ", data[0], data[1], data[2], data[3]);
      
        if(receievdCount == 2)
        {
            if(data[2] == 0x00)
            {
#ifdef DEBUG
                Serial.printf("Something is Blocking Sensor or Too Far!!\r\n");
#endif
                receievdCount = 0;
                distance = -1;
            }
        }
        else if(receievdCount == 3)
        {
            DATA_RESULT = (data[0] + data[1] + data[2]) & 0x00FF;
            if( data[3] == DATA_RESULT )
            {
                distance = (data[1] << 8) + data[2];
                if(distance > 30)
                {
#ifdef DEBUG
                    Serial.printf("Distance : %4.0f [ %3.0f ] cm\r\n", distance , distance/10);
#endif
                    distance = distance/10;
                }
                else
                {

                    distance = 0;
#ifdef DEBUG
                    Serial.printf("Below 30mm\r\n");
#endif
                }
            }
            else
            {                
                Serial.printf("Error %X %X %X \r\n", data[1], data[2], data[3]);
            }

            receievdCount = 0;
            for (int i = 0; i < 4; i++)
            {
                data[i] = 0;
            }
        }
      }
      myHWSerial->flush(); 
    }
    return (int)distance;
}

void Sonar::bubbleSort(unsigned char* target, unsigned char arraySize)
{
    unsigned char temp, i, j;
    unsigned char flag = 1;

    for (i = arraySize -1 ; flag&&i >= 1; i--)
    {
        flag = 0;
        for (j = 0; j < i; j++)
        {
            if(target[j] > target[j+1])
            {
                temp = target[j];
                target[j] = target[j+1];
                target[j+1] = temp;
                flag = 1; 
            }
        }
    }
}