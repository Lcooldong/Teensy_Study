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
uint64_t RS485_TimeOut = 0;

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
uint8_t* caculateModbusCRC(byte* _packet, uint8_t _length);
void printPacket();
void pollModbus();

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
        byte sendCoil[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
        
        for (int i = 0; i < 8; i++)
        {
          RS485.write(sendCoil[i]);
          Serial.printf("0x%X | ",sendCoil[i]);
          delay(1);
        }
        Serial.println();

        break;         
      }
  
      case '2':
      {
        byte sendCoil2[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
        for (int i = 0; i < 8; i++)
        {
          RS485.write(sendCoil2[i]);
          Serial.printf("0x%X | ",sendCoil2[i]);
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
          Serial.printf("[%d] : 0x%X\r\n", i,my_Coils[i]);
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
  pollModbus();

  indicator->breathe(5);
}





void initModbusCRC(CRC16* _crc)
{
  _crc->reset(CRC16_MODBUS_POLYNOME,
            CRC16_MODBUS_INITIAL,
            CRC16_MODBUS_XOR_OUT,
            CRC16_MODBUS_REV_IN,
            CRC16_MODBUS_REV_OUT);
}

uint8_t* caculateModbusCRC(byte* _packet, uint8_t _length) 
{
    static uint8_t _crcArray[2] = {0,};
    uint16_t _crcResult = 0;

    for (int i = 0; i < _length - 2; i++)
    {
      crc.add(_packet[i]);
      // Serial.printf("INPUT ==> 0x%02X\r\n", _packet[i]);
    }
    
    _crcResult = crc.calc();

    _crcArray[0] = _crcResult & 0b1111'1111;
    _crcArray[1] = _crcResult >> 8;

    // Serial.printf( "CRC_L : 0x%02X | CRC_H : 0x%02X => %04X\r\n", _crcArray[0], _crcArray[1] , CRC_RESULT);

    initModbusCRC(&crc);

    return (uint8_t*)_crcArray;
}


void printPacket()
{
  switch (readCount)
  {
  case 1:
    Serial.printf("[%d]FC         : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 2:
    Serial.printf("[%d]Address_H  : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 3:
    Serial.printf("[%d]Address_L  : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 4:
    Serial.printf("[%d]Coil_Value : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 5:
    Serial.printf("[%d]Coil_00    : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 6:
    Serial.printf("[%d]Checksum_L : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 7:
    Serial.printf("[%d]Checksum_H : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  }
  Serial.println("------------------------------------------------------");
}


void pollModbus()
{
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
        printPacket();
        readCount++;

        if(readCount == 8)
        {     
            for (int i = 0; i < (int)sizeof(recv); i++)
            {
              Serial.printf("0x%02X | ", recv[i]);
            }
            Serial.println();
                        
            uint8_t* crcReceived = caculateModbusCRC(recv, sizeof(recv));
            int startAddress = recv[2] * 256 + recv[3];
            Serial.printf("StartAddress : %d\r\n", startAddress);
            // Serial.printf("CRC ==> 0x%02X  | 0x%02X\r\n", crcReceived[0], crcReceived[1]);

            if(crcReceived[0] == recv[6] && crcReceived[1] == recv[7])
            {
              Serial.printf("Receive Complete \r\n");  

              switch (recv[1]){

              case READ_COILS:
              {
                
                byte targetCoil = 0;
                uint16_t numOfCoils = recv[5];
                
                for (uint8_t i = 0; i < numOfCoils; i++)
                {
                  targetCoil += my_Coils[startAddress + i] << i;
                }

                byte readCoils[] = {0x01, 0x01, 0x01, targetCoil, 0x00, 0x00};

                uint8_t* crcReadCoil = caculateModbusCRC(readCoils, sizeof(readCoils));
   
                readCoils[4] = crcReadCoil[0];
                readCoils[5] = crcReadCoil[1];
                
                
                Serial.println("\r\n===============ReadCoil RX===============");
                for (uint8_t i = 0; i < sizeof(readCoils); i++)
                {
                  Serial.printf("0x%X ", readCoils[i]);
                }
                Serial.println("\r\n===========================================");
                

                RS485.write(readCoils, sizeof(readCoils));
                RS485.flush(); 

                
                
                break;
              }
              case READ_HOLDING_REGISTERS:
              {
                Serial.printf("Read Holding Registers\r\n");
                
                break;
              }

              case WRITE_COIL:
              {
                if(recv[4] != 0)
                {
                  my_Coils[startAddress] = 0x01;  
                }
                else
                {
                  my_Coils[startAddress] = 0x00;
                }
                
                // for (uint8_t i = 0; i < sizeof(recv); i++)
                // {
                //   RS485.write(recv[i]);
                // }
                
                RS485.write(recv,sizeof(recv));
                RS485.flush(); 

                break;
              }
                
              

              default:
                break;
              }
              
              readCount = 0;
            }

        }
        else if(readCount > 8)
        {
          readCount = 0;
        }

      }
    }


  }
#endif
}