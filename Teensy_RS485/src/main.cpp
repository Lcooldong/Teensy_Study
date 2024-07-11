#include <Arduino.h>
#include "Indicator.h"
#include "config.h"
#include "Sonar.h"



#ifdef BOARD1

#elif defined(BOARD2)

#endif

Indicator* indicator = new Indicator();
Sonar* sonar1 = new Sonar(&Serial1);

unsigned long long sonarLastTime = 0;
unsigned long long testLastTime = 0;
int count = 0;

byte recv[6];
int recvLength = sizeof(recv)/sizeof(byte);

void setup() {
  Serial.begin(115200);
  SONAR1.begin(9600);
  SONAR2.begin(9600);
  SONAR3.begin(9600);
  SONAR4.begin(9600);
  RS485.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  
  if(millis() - testLastTime > 1000)
  {
    testLastTime = millis();
    // Serial.printf(" Size : %d\r\n" );
    count++;
    Serial.printf(" C : %d\r\n", count );
    RS485.printf(" C5 : %d\r\n", count);
  }

  if(Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case '1':
      RS485.println("1234");
      Serial.println("press 1");
      break;
    
    default:
      break;
    }
  }
  // PC 에서 요청 받을 경우 동작
  // if(RS485.available())
  // {
    
  //   }
  //   // 명령어 수신
  //   RS485.readBytes(recv, recvLength);

  //   // memcpy();
  // }
  
  
  int distance = sonar1->getDistance();

  if(millis() - sonarLastTime > 50)
  {
    sonarLastTime = millis();
    Serial.printf("D : %d\r\n", distance);
  }
  


 
  indicator->breathe(5); 
}





