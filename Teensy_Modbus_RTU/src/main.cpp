#include <Arduino.h>
#include "CRC16.h"
#include "CRC.h"
#include "Indicator.h"
#include <SoftwareSerial.h>


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
#define TIME_OUT 1000
#define DEFAULT_LENGTH 8

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
int16_t inputRegisters[ADDRESS_LENGTH] = {0,};
int16_t holdingRegisters[ADDRESS_LENGTH] = {32767, 32766,};

typedef enum
{
  READ_COILS                    = 0x01,
  READ_DISCRETE_INPUTS          = 0x02,
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

byte recv[ADDRESS_LENGTH] = {0,};
uint64_t sendCount = 0;

struct __attribute__((packed)){
  byte slaveID;
  byte fc;
  uint16_t coilNum;
  uint16_t cmd;
  byte CRC16_L;
  byte CRC16_H;
}FC05;


bool state = true;
bool recevicedFlag = false;
bool whileReadingFlag = false;

void initModbusCRC(CRC16* _crc);
uint8_t* caculateModbusCRC(byte* _packet, uint8_t _length);
void printReadPacket();
void printPacket(uint16_t _length);
void pollModbus();
int getFunctionCode();
void readCoils(uint16_t _startAddress, uint16_t _count);
void writeMultipleCoils(uint16_t _startAddress, uint16_t _count, uint16_t _length);
void readHoldingRegisters(uint16_t _startAddress, uint16_t _count);

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
  // pollModbus();
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


void printReadPacket()
{
  switch (readCount)
  {
  case 1:
    Serial.printf("[%d]FC              : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 2:
    Serial.printf("[%d]Address_H       : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 3:
    Serial.printf("[%d]Address_L       : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 4:
    Serial.printf("[%d]Coil_Value      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 5:
    Serial.printf("[%d]Register_Length : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 6:
    Serial.printf("[%d]Checksum_L      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 7:
    Serial.printf("[%d]Checksum_H      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  }
  Serial.println("------------------------------------------------------");
}

void printPacket(uint16_t _length)
{
  for (int i = 0; i < _length; i++)
  {
    Serial.printf("0x%02X | ", recv[i]);
  }
  Serial.println();
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

      uint16_t coil_register_count = 0;
      uint16_t startAddress = 0;

      if(ch != -1)
      {
        Serial.printf("READ : 0x%02X\r\n", ch);
        whileReadingFlag = true;
        recevicedFlag = false;
        RS485_TimeOut = 0;
      }

      if(ch == SLAVE_ID && readCount == 0)
      {
        // Serial.printf("[%d]ID         : 0x%X\r\n", readCount, SLAVE_ID);
        recv[0] = SLAVE_ID;
        readCount = 1;
      }
      else if(readCount > 0 )
      {
        recv[readCount] = ch;        
        readCount++;

      }



      if(readCount >= DEFAULT_LENGTH)  // Write Single Coil, Register, Read
      {   
          
          uint8_t fc = recv[1];
          uint8_t* crcReceived;
          startAddress = recv[2] * 256 + recv[3];
          coil_register_count = (recv[4] << 8) + recv[5];
          uint8_t data_count = recv[6];
          uint16_t data_length = data_count + DEFAULT_LENGTH + 1;

          if( (fc > 6) && (readCount == data_length))
          {
            // Serial.printf("[%d]NUM Data : %d [%d]\r\n", readCount, num_data_length, data_length);
            crcReceived = caculateModbusCRC(recv, data_length);
          }
  
          else
          {
            crcReceived = caculateModbusCRC(recv, DEFAULT_LENGTH);
          }

          
          

          // Serial.printf("CRC ==> 0x%02X  | 0x%02X\r\n", crcReceived[0], crcReceived[1]);

          if(crcReceived[0] == recv[readCount - 2] && crcReceived[1] == recv[readCount -1])
          {
            getFunctionCode();
            printPacket(readCount);

            switch (fc){

            case READ_COILS:
            {
              readCoils(startAddress, coil_register_count);               
              break;
            }

            case READ_DISCRETE_INPUTS:
            {

              break;
            }


            case READ_HOLDING_REGISTERS:
            {

              readHoldingRegisters(startAddress, coil_register_count);
              break;
            }
            
            case READ_INPUT_REGISTERS:
            {

              break;
            }

            case WRITE_SINGLE_COIL:
            {
              if(recv[4] != 0)
              {
                my_Coils[startAddress] = 0x01;  
              }
              else
              {
                my_Coils[startAddress] = 0x00;
              }
              
              Serial.printf("WRItE : 0x%X\r\n", my_Coils[startAddress]);
              RS485.write(recv, DEFAULT_LENGTH);
              RS485.flush(); 
              break;
            }        

            case WRITE_MULTIPLE_COILS:
            {
              writeMultipleCoils(startAddress, coil_register_count, data_count);
              break;
            }
              

            case WRITE_MULTIPLE_REGISTERS:
            {
              
              break;
            }

            default:
              break;
            } 

            Serial.printf("Receive Complete \r\n");
            recevicedFlag = true;
            readCount = 0;
          }
          whileReadingFlag = false;
      }

      if(recevicedFlag)
      {
        startAddress = recv[2] * 256 + recv[3];
        Serial.printf("StartAddress : %d\r\n", startAddress);
      }

      
    }
  }
  else
  {
    if((millis() - RS485_TimeOut > TIME_OUT)  && !whileReadingFlag)
    {
      RS485_TimeOut = millis();
      // Serial.printf("RS485 - TIMEOUT %d\r\n", TIME_OUT);
      readCount = 0;
    }
  }


#endif
}

int getFunctionCode()
{
   switch (recv[1]){

    case READ_COILS:
    {
      Serial.printf("[Read Coils]\r\n");
      return READ_COILS;
      break;
    }

    case READ_DISCRETE_INPUTS:
    {
      Serial.printf("[Read Discrete Inputs]\r\n");
      return READ_DISCRETE_INPUTS;
      break;
    }


    case READ_HOLDING_REGISTERS:
    {
      Serial.printf("[Read Holding Registers]\r\n");

      return READ_HOLDING_REGISTERS;
      break;
    }
    
    case READ_INPUT_REGISTERS:
    {
      Serial.printf("[Read Input Registers]\r\n");
      return READ_INPUT_REGISTERS;
      break;
    }

    case WRITE_SINGLE_COIL:
    {
      Serial.printf("[Write Single Coils]\r\n");
      return WRITE_SINGLE_COIL;  
      break;
    }        

    case WRITE_MULTIPLE_COILS:
    {
      Serial.printf("[Write Multiple Coils]\r\n");
      return WRITE_MULTIPLE_COILS;
      break;
    }
      

    case WRITE_MULTIPLE_REGISTERS:
    {
      Serial.printf("[Write Multiple Registers]\r\n");
      return WRITE_MULTIPLE_REGISTERS;
      break;
    }

    default:
      return -1;
      break;
    } 
}

void readCoils(uint16_t _startAddress, uint16_t _count)
{
  uint16_t readingCount = (_count / DEFAULT_LENGTH) + 1;  // % 가 나머지
  byte readingCoilsByte[readingCount] = {0, };
  uint8_t sendCount = 1;  // 기본 자리 1


  Serial.printf("Read Return Data Count : %d\r\n", readingCount);

  

  for (uint16_t i = 0; i < readingCount; i++)
  {  
    byte _temp = 0;
    for (uint16_t j = 0; j < _count; j++)
    {
      _temp += my_Coils[_startAddress + (j % DEFAULT_LENGTH) + (i * DEFAULT_LENGTH)] << j;
    }
    readingCoilsByte[i] = _temp;
    
    
    Serial.printf("RETURN COIL BYTE[%d] : 0x%02X\r\n", i, _temp);
  }

  if(_count > DEFAULT_LENGTH)
  {
    for (uint16_t i = 1; i < readingCount; i++)
    {
      if((readingCoilsByte[i]) >= ( 0x01 ))
      {
        sendCount++;
      }
    }
  }
  
  

  Serial.printf("SendCount : [%d]\r\n", sendCount);


  byte sendCoils[sendCount + 5] = {0,};
  
  sendCoils[0] = SLAVE_ID;
  sendCoils[1] = READ_COILS;
  sendCoils[2] = sendCount;
  for (uint16_t i = 0; i < sendCount; i++)
  {
    sendCoils[i + 3] = readingCoilsByte[i];
  }
  
                
  uint8_t* crcReadCoil = caculateModbusCRC(sendCoils, sizeof(sendCoils));

  sendCoils[sizeof(sendCoils) - 2] = crcReadCoil[0];
  sendCoils[sizeof(sendCoils) - 1] = crcReadCoil[1];
  
  
  Serial.println("\r\n===============ReadCoil RX===============");
  for (uint8_t i = 0; i < sizeof(sendCoils); i++)
  {
    Serial.printf("0x%02X ", sendCoils[i]);
  }
  Serial.println("\r\n===========================================");
  

  RS485.write(sendCoils, sizeof(sendCoils));
  RS485.flush(); 
}

void writeMultipleCoils(uint16_t _startAddress, uint16_t _count, uint16_t _length)
{
  Serial.printf("Write Multiple COils [S %d |C %d |NB %d] \r\n", _startAddress, _count, _length);

  uint16_t multipleCoils[_length] = {0, };
  uint32_t totalCoils = 0;


  Serial.printf("Multiple Coils => ");
  for (uint8_t i = 0; i < _length; i++)
  {
    multipleCoils[i] = recv[DEFAULT_LENGTH - 1 + i];
    totalCoils += multipleCoils[i] << (i * DEFAULT_LENGTH);
    Serial.printf("0x%X | ", multipleCoils[i]);
  }
  Serial.println();

  Serial.printf("Total : 0x%X\r\n", totalCoils);

  
  for (int i = 0; i < _count; i++)
  {
    uint8_t _bit = (((totalCoils) >> (i)) & 0x01);
    my_Coils[_startAddress + i] = _bit;
    
    Serial.printf("[%d] %d, ", _startAddress + i, _bit);
  }
  Serial.println();

  byte returnArray[DEFAULT_LENGTH] = {0,};
  for (uint8_t i = 0; i < DEFAULT_LENGTH - 2; i++)
  {
    returnArray[i] = recv[i];
  }

  uint8_t* crcMultipleCoil = caculateModbusCRC(returnArray, sizeof(returnArray));

  returnArray[DEFAULT_LENGTH - 2] = crcMultipleCoil[0];
  returnArray[DEFAULT_LENGTH - 1] = crcMultipleCoil[1];
  
  RS485.write(returnArray, DEFAULT_LENGTH);
  RS485.flush();  
}

void readHoldingRegisters(uint16_t _startAddress, uint16_t _count)
{
                
  uint8_t holdingBytes = _count * 2;
  byte readholdingRegisters[holdingBytes + 5] = {0,};

  readholdingRegisters[0] = SLAVE_ID;
  readholdingRegisters[1] = READ_HOLDING_REGISTERS;
  readholdingRegisters[2] = holdingBytes;

  Serial.printf("READING HOLDING : [%d]\r\n", holdingBytes);



  for (uint16_t i = 0; i < _count; i++)
  {
    readholdingRegisters[i*2 + 3] = holdingRegisters[_startAddress + i] >> 8;
    readholdingRegisters[i*2 + 4] = holdingRegisters[_startAddress + i] & 0xFF;
    Serial.printf("HOLDING REGISTERS : 0x%02X | 0x%02X\r\n", readholdingRegisters[i*2 + 3], readholdingRegisters[i*2 + 4]);
  }
  
  

  uint8_t* crcReadCoil = caculateModbusCRC(readholdingRegisters, sizeof(readholdingRegisters));

  readholdingRegisters[sizeof(readholdingRegisters) - 2] = crcReadCoil[0];
  readholdingRegisters[sizeof(readholdingRegisters) - 1] = crcReadCoil[1];

  Serial.println("\r\n===============ReadCoil RX===============");
  for (uint8_t i = 0; i < sizeof(readholdingRegisters); i++)
  {
    Serial.printf("0x%02X ", readholdingRegisters[i]);
  }
  Serial.println("\r\n===========================================");

  RS485.write(readholdingRegisters, sizeof(readholdingRegisters));
  RS485.flush(); 
}