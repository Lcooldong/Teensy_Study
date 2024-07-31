#include <Arduino.h>
#include "CRC16.h"
#include "CRC.h"
#include "Indicator.h"
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

#define DEBUG
#define MASTER
#define SLAVE
#define RECEIVE
#define SLAVE_ID 0x01

// FUNCTION CODE
#define WRITE_COIL 0x05
#define COIL_TRUE 0xFF
#define COIL_FALSE 0x00
#define ADDRESS_LENGTH 65534

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
uint64_t returnCount = 0;
int count = 0;
uint8_t readCount = 0;

uint16_t CRC_RESULT;
uint8_t CRC_L;
uint8_t CRC_H;
char buffer[32];

// ID, FC, ( Address , 2 byte) (data, 2 byte)  CRC : 3A8C (HL)
uint8_t sample[] = {SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00, 0x00, 0x00};
// uint8_t sampleFC05[] = {0x06 , SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00};
// uint8_t sampleFC05[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06 , SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00};
uint8_t sampleFC05[] = {SLAVE_ID, WRITE_COIL, 0x00, 0x00, COIL_TRUE, 0x00, 0x00, 0x00};

uint8_t discreteInputs[ADDRESS_LENGTH] = {0,};
uint8_t my_Coils[ADDRESS_LENGTH] = {0,};
short inputRegisters[ADDRESS_LENGTH] = {0,};
short holdingRegisters[ADDRESS_LENGTH] = {0,};

typedef enum
{
  READ_COILS                    = 0x01,
  READ_CONTACTS                 = 0x02,
  READ_HOLDING_REGISTERS        = 0X03,
  READ_INPUT_REGISTERS          = 0X04,
  WRITE_SINGLE_COIL             = 0x05,
  WRITE_SINGLE_REGISTER         = 0X06,
  WRITE_MULTIPLE_COILS          = 0x0F,
  WRITE_MULTIPLE_REGISTERS      = 0x10, 
  MASK_WRITE_REGISTER           = 0x16, 
  READ_WRITE_MULTIPLE_REGISTERS = 0x17,
  READ_FIFO_QUEUE               = 0x18

}FUNCTION_CODE;

byte recv[8] = {0,};
uint64_t sendCount = 0;

struct __attribute__((packed)){
  byte slaveID;
  byte fc;
  uint16_t coilNum;
  uint16_t cmd;
  byte CRC16_L;
  byte CRC16_H;
}FC05;


ModbusMaster node;
bool state = true;

void initModbusCRC(CRC16* _crc);

void setup() {
  Serial.begin(115200);
  RS485.begin(9600);
  node.begin(1, RS485);

  pinMode(LED_BUILTIN, OUTPUT);
#ifdef DEBUG
  while(!Serial){}; // Wait for serial port connect
#endif
  
  Serial.println(__FILE__);
  RS485.printf("Start RS485\r\n");
  
  // uint8_t arr[] = { 0xFE, 0x10, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x00}; 

  // for (int i = 0; i < (int)sizeof(sample) - 2; i++)
  // {
  //   crc.add(sample[i]);
  // }
  
  // uint16_t CRC_RESULT = crc.calc();

  // CRC_L = CRC_RESULT & 0b1111'1111;
  // CRC_H = CRC_RESULT >> 8;

  // Serial.printf( "CRC_L : 0x%X | CRC_H : 0x%X => %X\r\n", CRC_L, CRC_H , CRC_RESULT);

  // initModbusCRC(&crc);

  Serial.println("--Start--");
}



void loop() {

  if(Serial.available())
  {
    char cmd = Serial.read();

    switch (cmd)
    {
      case '1':
      {
        byte sendCoil[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
        
        for (int i = 0; i < 8; i++)
        {
          RS485.write(sendCoil[i]);
          Serial.printf("0x%X | ",sendCoil[i]);
          delay(1);
        }
        Serial.println();

        int cnt1 = 0;
        while (cnt1 < 8)
        {
          char c1 = RS485.read();
          Serial.printf("+++ 0x%X\r\n", c1);
          cnt1++;
          delay(1);
        }
        break;         
      }
  
      case '2':
      {
        byte sendCoil2[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
        for (int i = 0; i < 8; i++)
        {
          RS485.write(sendCoil2[i]);
          Serial.printf("0x%X | ",sendCoil2[i]);
        }
        Serial.println();

        // int cnt2 = 0;

        break;
      }

    }

  }



  // Send
  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;
    // Serial.printf("[%d]\r\n", count);
    // uint8_t resultMain;
    // // resultMain = node.readCoils(0x0, 1);
    // resultMain = node.writeSingleCoil(0x03, 0xFF);
    // if(resultMain == node.ku8MBSuccess)
    // {
    //   Serial.println(node.getResponseBuffer(0x01));
    // }

    
  
    // byte returnBytes[8];
    // RS485.readBytes(returnBytes, sizeof(returnBytes));
    // for (uint8_t i = 0; i < sizeof(returnBytes); i++)
    // {
    //   Serial.printf("0x%X || ", returnBytes[i]);
    // }
    // Serial.println();

    // for (int i = 0; i < (int)sizeof(returnBytes) - 2; i++)
    // {
    //   crc.add(returnBytes[i]);
    // }
    
    // uint16_t CRC_RESULT = crc.calc();

    // CRC_L = CRC_RESULT & 0b1111'1111;
    // CRC_H = CRC_RESULT >> 8;

    // Serial.printf( "CRC_L : 0x%X | CRC_H : 0x%X || [%X]\r\n", CRC_L, CRC_H , CRC_RESULT);

    // initModbusCRC(&crc);
    
   
    
  }

  

#ifdef RECEIVE
  // Receive
  if(RS485.available())
  {
    
    if(millis() - rs485Time > 1)
    {
      rs485Time = millis();
    
      int ch = RS485.read();

      if(ch != -1)
      {
        Serial.printf("READ : 0x%X\r\n", ch);
      }
      
      if(ch == SLAVE_ID && readCount == 0)
      {
        Serial.printf("[%d]ID         : 0x%X\r\n", readCount, SLAVE_ID);
        recv[0] = SLAVE_ID;
        readCount++;
      }
      else if(readCount > 0 && readCount < 8) 
      {  
        recv[readCount] = ch;
        // Serial.printf( "[%d]->0x%X\r\n" , readCount, recv[readCount]);
        switch (readCount)
        {
        case 1:
          Serial.printf("[%d]FC         : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 2:
          Serial.printf("[%d]Address_H  : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 3:
          Serial.printf("[%d]Address_L  : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 4:
          Serial.printf("[%d]Coil_Value : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 5:
          Serial.printf("[%d]Coil_00    : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 6:
          Serial.printf("[%d]Checksum_L : 0x%X\r\n", readCount, recv[readCount]);
          break;
        case 7:
          Serial.printf("[%d]Checksum_H : 0x%X\r\n", readCount, recv[readCount]);
          break;
        }
        readCount++;

        if(readCount == 8)
        {     
            for (int i = 0; i < (int)sizeof(recv); i++)
            {
              Serial.printf("0x%X | ", recv[i]);
            }
            Serial.println();
            

            for (int i = 0; i < (int)sizeof(recv) - 2; i++)
            {
              crc.add(recv[i]);
            }
            
            CRC_RESULT = crc.calc();

            CRC_L = CRC_RESULT & 0b1111'1111;
            CRC_H = CRC_RESULT >> 8;

            Serial.printf( "CRC_L : 0x%02X | CRC_H : 0x%02X => %04X\r\n", CRC_L, CRC_H , CRC_RESULT);

            initModbusCRC(&crc);

            if(CRC_L == recv[6] && CRC_H == recv[7])
            {
              Serial.printf("Receive Complete \r\n");  

              switch (recv[1])
              {
              case READ_COILS:
              {
                // byte my_Coils[] = {0x01, 0x00, 0x01};
                byte targetCoil = my_Coils[recv[3]];
                byte readCoils[] = {0x01, 0x01, 0x01, targetCoil, 0x00, 0x00};

                for (int i = 0; i < (int)sizeof(readCoils) - 2; i++)
                {
                  crc.add(readCoils[i]);
                }
                
                CRC_RESULT = crc.calc();

                CRC_L = CRC_RESULT & 0b1111'1111;
                CRC_H = CRC_RESULT >> 8;

                Serial.printf( "CRC_L : 0x%02X | CRC_H : 0x%02X => %04X\r\n", CRC_L, CRC_H , CRC_RESULT);

                readCoils[sizeof(readCoils) - 2] = CRC_L;
                readCoils[sizeof(readCoils) - 1] = CRC_H;

                initModbusCRC(&crc);
                
                RS485.write(readCoils, sizeof(readCoils));
                RS485.flush(); 

                readCount = 0;
                break;
              }

              case WRITE_COIL:
              {
                if(recv[4] != 0)
                {
                  my_Coils[recv[3]] = 0x01;         
                }
                else
                {
                  my_Coils[recv[3]] = 0x00;
                }
                
                // for (uint8_t i = 0; i < sizeof(recv); i++)
                // {
                //   RS485.write(recv[i]);
                // }
                
                RS485.write(recv,sizeof(recv));
                RS485.flush(); 

                // for (int i = 0; i < (int)sizeof(recv); i++)
                // {
                //   recv[i] = 0;
                // }
                readCount = 0;
                break;
              }
                
              

              default:
                break;
              }
            }

        }

      }
    }


  }
#endif
  

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