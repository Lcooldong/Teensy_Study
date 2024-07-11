#ifndef __SONAR_H__
#define __SONAR_H__

#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <iostream>
#include <vector>

class Sonar
{
private:
    HardwareSerial* myHWSerial;
    SoftwareSerial* mySWSerial;

    const int MAX_DISTANCE = 2100;  // 2100 mm
    unsigned long long sonarLastTime = 0;
    unsigned long long UART_Interval = 1;
    unsigned char data[4]={};
    float distance;
    

    unsigned char receivedData = 0;
    unsigned int receievdCount = 0;
    unsigned char DATA_RESULT = 0;


    void bubbleSort(unsigned char* target, unsigned char arraySize);

public:

    Sonar(SoftwareSerial* _serial);
    Sonar(HardwareSerial* _serial);
    ~Sonar();

    void initSonar();
    int getDistance();
    

};



#endif