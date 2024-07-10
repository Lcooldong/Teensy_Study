#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define SONAR_COUNT 8
#define MIN_DISTANCE 10

uint64_t sonarTime = 0;
uint64_t ledBlinkTime = 0;
bool ledState = false;
//unsigned char data[4] = {};
unsigned char digit[3] = {};

uint8_t sonarSelect = 0;
//float distance[SONAR_COUNT];


typedef struct sonar
{
  HardwareSerial* targetSerial;
  unsigned char data[4];
  float distance;
}SONAR;


SONAR sonarSensor[8];

void SelectUART(uint8_t portNumber);
int blink(uint32_t _delaY);
void getSonarDistance(int _num);



void setup() {
  Serial.begin(115200);

  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  Serial4.begin(9600);
  Serial5.begin(9600);
  Serial6.begin(9600);
  Serial7.begin(9600);
  Serial8.begin(9600);
  
  SPI1.begin();
  

  pinMode(LED_BUILTIN, OUTPUT);

  sonarSensor[0].targetSerial = &Serial1;
  sonarSensor[1].targetSerial = &Serial2;
  sonarSensor[2].targetSerial = &Serial3;
  sonarSensor[3].targetSerial = &Serial4;
  sonarSensor[4].targetSerial = &Serial5;
  sonarSensor[5].targetSerial = &Serial6;
  sonarSensor[6].targetSerial = &Serial7;
  sonarSensor[7].targetSerial = &Serial8;

  for (size_t i = 0; i < SONAR_COUNT; i++)
  {
    sonarSensor[i].distance = 4500;
  }
  

  Serial.printf("Start Teensy Sonar");

  delay(1000);
}



void loop() {
  


  
  if(millis() - sonarTime > 100)
  {
    sonarTime = millis();

    //  Serial.printf("Sonar[%d] : %.2f cm\r\n", 1,sonarSensor[1].distance);
    
    for (int j = 0; j < SONAR_COUNT; j++)
    {       
      getSonarDistance(j);
    }

    Serial.println();
  }

  
 
  blink(1000);

}


void SelectUART(uint8_t portNumber) // 0 ~ 7
{
  uint8_t num = 0, quotient, remainder;

  do
  {
    quotient = portNumber / 2;
    remainder = portNumber - (quotient * 2);
    if(num < 3)
    {
      digit[num++] = remainder;
    }
    portNumber = quotient;
  } while (quotient != 0);
  
  // Serial.printf(":: %d |  %d | %d  ", digit[0], digit[1], digit[2]);

}


int blink(uint32_t _delaY)
{
  if(millis() - ledBlinkTime > _delaY)
  {
    ledBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);

    return 1;
  }
  else
  {
    return -1;
  }
}

void getSonarDistance(int _num)
{
   do{

      for(int i=0;i<4;i++)
      {
        sonarSensor[_num].data[i]=sonarSensor[_num].targetSerial->read();
        delay(1);
      }
    }while(sonarSensor[_num].targetSerial->read()==0xFF);


    sonarSensor[_num].targetSerial->flush(); 



    if(sonarSensor[_num].data[0]==0xFF)
    {
      int sum;
      sum=(sonarSensor[_num].data[0]+sonarSensor[_num].data[1]+sonarSensor[_num].data[2])&0x00FF;

      if(sum==sonarSensor[_num].data[3])
      {
        Serial.printf("[%d]:", _num);
        sonarSensor[_num].distance=(sonarSensor[_num].data[1]<<8)+sonarSensor[_num].data[2];
        if(sonarSensor[_num].distance >= MIN_DISTANCE)
          {
            // Serial.printf("%.2f cm", sonarSensor[_num].distance/10);
              Serial.printf("%5.2f",sonarSensor[_num].distance/10);
          }
          else 
          {
            Serial.print("Below the lower limit");
          }
      }
      // else Serial.print("ERROR");
    }
    else
    {
      // Serial.print("NOT");
    }

}