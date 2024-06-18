#include <Arduino.h>
#include <HardwareSerial.h>

#define SONAR_COUNT 8

const int EN_PIN = 14;
const int S1_PIN = 15;
const int S2_PIN = 18;
const int S3_PIN = 19;

uint64_t sonarTime = 0;
uint64_t ledBlinkTime = 0;
bool ledState = false;
unsigned char data[4] = {};
unsigned char digit[3] = {};

uint8_t sonarSelect = 0;
float distance[SONAR_COUNT];

void SelectUART(uint8_t portNumber);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  pinMode(EN_PIN, OUTPUT);  // Default : Not Connected
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);  
  pinMode(S3_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);


  

  // RX3, TX3 => 010
  digitalWrite(S1_PIN, LOW);
  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, LOW);


  // analogWrite(S1_PIN, 255);


  Serial.printf("Start Teensy Sonar");

  delay(1000);
}



void loop() {
  

  
  if(millis() - sonarTime > 100)
  {
    SelectUART(sonarSelect);
    if(sonarSelect >= SONAR_COUNT) sonarSelect = 0;
    sonarTime = millis();
    
    do{
    for(int i=0;i<4;i++)
    {
      data[i]=Serial2.read();
    }
    }while(Serial2.read()==0xff);

    //Serial2.flush(); // Speed Down 

    if(data[0]==0xff)
    {
      int sum;
      sum=(data[0]+data[1]+data[2])&0x00FF;
      Serial.printf("[%d] : ", sonarSelect);
      if(sum==data[3])
      {
        distance[sonarSelect]=(data[1]<<8)+data[2];
        if(distance[sonarSelect]>30)
          {
            Serial.printf("distance[%d]= %f cm\r\n",  sonarSelect, distance[sonarSelect]/10);    
          }
          else 
          {
            // Serial.println("Below the lower limit");
          }
      }
      // else Serial.println("ERROR");
    }
    sonarSelect++;
  }

  
 
  if(millis() - ledBlinkTime > 1000)
  {
    ledBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);

  }

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

  digitalWrite(S1_PIN, digit[0]);
  digitalWrite(S2_PIN, digit[1]);
  digitalWrite(S3_PIN, digit[2]);
}
