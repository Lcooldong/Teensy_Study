#include <Arduino.h>
#include "Indicator.h"
#include <SoftwareSerial.h>
#include "MyModbus.h"

#define DEBUG
#define MASTER
#define SLAVE
#define SLAVE_ID 0x01



SoftwareSerial RS485(0, 1); // RX,TX
//SoftwareSerial RS485(7, 8);
//SoftwareSerial RS485(15, 14);
//SoftwareSerial RS485(16, 17);
//SoftwareSerial RS485(21, 20);
//SoftwareSerial RS485(25, 24);
//SoftwareSerial RS485(28, 29);
//SoftwareSerial RS485(34, 35); // Teensy 4.1 only


uint64_t lastTime = 0;
int count = 0;
bool state = true;
// ID, FC, ( Address , 2 byte) (data, 2 byte)  CRC : 3A8C (HL)
uint8_t sample[] = {SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00, 0x00, 0x00};
// uint8_t sampleFC05[] = {0x06 , SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00};
// uint8_t sampleFC05[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06 , SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00};
uint8_t sampleFC05[] = {SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00, 0x00, 0x00};


Indicator* indicator = new Indicator();
MyModbus* myModbus = new MyModbus(&RS485, SLAVE_ID);

void setup() {
  Serial.begin(115200);
  RS485.begin(9600);



  pinMode(LED_BUILTIN, OUTPUT);
#ifdef DEBUG
  while(!Serial){}; // Wait for serial port connect
#endif
  
  Serial.println(__FILE__);
  RS485.printf("Start RS485\r\n");
  
  Serial.println("--Start--");
}



void loop() {

  // Test
  if(Serial.available())
  {
    char cmd = Serial.read();

    switch (cmd)
    {
      case '1':
      {
        byte readCoil[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
        
        for (int i = 0; i < 8; i++)
        {
          RS485.write(readCoil[i]);
          Serial.printf("0x%X | ",readCoil[i]);
          delay(1);
        }
        Serial.println();

        break;         
      }
  
      case '2':
      {
        byte sendCoil[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
        for (int i = 0; i < 8; i++)
        {
          RS485.write(sendCoil[i]);
          Serial.printf("0x%X | ",sendCoil[i]);
          delay(1);
        }
        Serial.println();

        // int cnt2 = 0;

        break;
      }

      case '3':
        Serial.println("***********myCoils*************");
        for (uint8_t i = 0; i < 10; i++)
        {
          Serial.printf("[%d] : 0x%X\r\n", i, myModbus->my_Coils[i]);
        }
        Serial.println("******************************");

        break;

    }

  }



  // Send
  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;   
    
  }

  myModbus->pollModbus();
  indicator->breathe(5);
}





