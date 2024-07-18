#include <Arduino.h>
#include "CRC16.h"
#include "CRC.h"
#include "Indicator.h"
#include <SoftwareSerial.h>


#define DEBUG
#define MASTER
#define SLAVE
#define SLAVE_ID 0x01

// FUNCTION CODE
#define WRITE_COIL 0x05
#define COIL_TRUE 0xFF
#define COIL_FALSE 0x00


CRC16 crc(CRC16_MODBUS_POLYNOME,
          CRC16_MODBUS_INITIAL,
          CRC16_MODBUS_XOR_OUT,
          CRC16_MODBUS_REV_IN,
          CRC16_MODBUS_REV_OUT);


SoftwareSerial RS485(0, 1); // RX,TX
//SoftwareSerial RS485(7, 8);
//SoftwareSerial RS485(15, 14);
//SoftwareSerial RS485(16, 17);
//SoftwareSerial RS485(21, 20);
//SoftwareSerial RS485(25, 24);
//SoftwareSerial RS485(28, 29);
//SoftwareSerial RS485(34, 35); // Teensy 4.1 only
Indicator* indicator = new Indicator();

uint64_t lastTime = 0;
uint64_t rs485Time = 0;
int count = 0;


uint8_t CRC_L;
uint8_t CRC_H;
char buffer[32];

// ID, FC, ( Address , 2 byte) (data, 2 byte)  CRC : 3A8C (HL)
uint8_t sample[] = {SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00, 0x00, 0x00};


void initModbusCRC(CRC16* _crc);

void setup() {
  Serial.begin(115200);
  RS485.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef DEBUG
  while(!Serial){}; // Wait for serial port connect
#endif
  
  Serial.println(__FILE__);

  
  uint8_t arr[] = { 0xFE, 0x10, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x00}; 

  for (int i = 0; i < (int)sizeof(sample) - 2; i++)
  {
    crc.add(sample[i]);
  }
  
  uint16_t CRC_RESULT = crc.calc();

  CRC_L = CRC_RESULT & 0b1111'1111;
  CRC_H = CRC_RESULT >> 8;

  Serial.printf( "CRC_L : 0x%X | CRC_H : 0x%X => %X\r\n", CRC_L, CRC_H , CRC_RESULT);

  initModbusCRC(&crc);

  Serial.println("--Start--");
}

void loop() {

  // Send
  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;


    for (int i = 0; i < (int)sizeof(sample) - 2; i++)
    {
      crc.add(sample[i]);
    }

    uint16_t CRC_RESULT = crc.calc();
    Serial.printf("RESULT :  0x%X\r\n", CRC_RESULT);
    sample[sizeof(sample) - 2] = CRC_RESULT & 0b1111'1111;
    sample[sizeof(sample) - 1] = CRC_RESULT >> 8;

    RS485.write(sample, sizeof(sample));

    for (int i = 0; i < (int)sizeof(sample); i++)
    {
      Serial.printf("0x%X |",sample[i]);
    }
    Serial.println();

    // String text;
    // for (int i = 0; i < (int)sizeof(sample); i++)
    // {
    //   text += sample[i] + "|";  
    // }
    
    // // snprintf(buffer, sizeof(buffer) , "%d", count);
    // Serial.printf("[%d] %s\r\n", count, text);
    
    // RS485.printf("CNT : %d\r\n", count);
    RS485.flush();
    initModbusCRC(&crc);
    // crc.reset();
  }


  // Receive
  if(RS485.available())
  {
    if(millis() - rs485Time > 1)
    {
      rs485Time = millis();

    }
  }
  

  indicator->breathe(5);
}


void writeCoil(uint8_t _address)
{
  uint8_t dataToSend[] = {0, };
}


void initModbusCRC(CRC16* _crc)
{
  _crc->reset(CRC16_MODBUS_POLYNOME,
            CRC16_MODBUS_INITIAL,
            CRC16_MODBUS_XOR_OUT,
            CRC16_MODBUS_REV_IN,
            CRC16_MODBUS_REV_OUT);
}