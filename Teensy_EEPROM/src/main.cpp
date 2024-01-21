#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>


#include <EEPROM.h>

#define COUNT_START 0
#define COUNT_END   4
// #define WRITABLE_COUNT 95000
#define WRITABLE_COUNT 1234567890

uint32_t resultValue = 0;
uint8_t writeCount[ COUNT_END - COUNT_START] = {0,};
uint8_t writeCountLength = sizeof(writeCount)/sizeof(writeCount[0]);
uint8_t reverseLength = writeCountLength - 1;
uint8_t* intArray;
uint32_t testValue = 0;

//FUNCTION
void initEEPROM();
void setAllEEPROM(int _value);
uint32_t HexToInt(uint8_t* _hexArray);
uint8_t* IntToHex(uint32_t _int);
bool writeIntToEEPROM(uint8_t _num, uint32_t _int);
uint8_t* writeEEPROMIntToHex(uint32_t _int, uint8_t _num, bool writable);
void printEEPROM(uint8_t _start, uint8_t _end);

static void blink(void*) {
    while (true) {
        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
        ::vTaskDelay(pdMS_TO_TICKS(500));

        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}


uint64_t systemCount = 0;
uint16_t serialInterval = 1000;
static void tickTock(void*) {
    while (true) {
        ::Serial.printf("Flow : %lu\r\n", systemCount++);
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}

static void uartTask(void*)
{
    while (true)
    {
        char text = Serial.read();
        
        switch (text)
        {
        case '1':
            printEEPROM(0, 10);
            break;

        case '2':
            
            for (int i = 0; i < writeCountLength; i++)
            {
              writeCount[i] = EEPROM.read(i);
              resultValue |= (writeCount[reverseLength - i] << 8*i);
              
            }
            Serial.printf("Result : %d\r\n", resultValue);
            break;

        case '3':
            
            Serial.println("Press 3");

            intArray = writeEEPROMIntToHex(1237, 1, true); // 1234 를 1 번 자리에 쓰기

            for (int i = 0; i < 4; i++)
            {
              Serial.printf("3 Array [%d] %d\r\n", i , intArray[i]);
            }
            Serial.println();

            resultValue = HexToInt(intArray);

            Serial.printf("RESULT Value3 : %d\r\n", resultValue);
            break;
        
        case '4':
            resultValue = HexToInt(writeCount);
            Serial.printf("RESULT Value4 : %d\r\n", resultValue);
            break;
        
        case '5':
            intArray = IntToHex(5234);
            for (int i = 0; i < 4; i++)
            {
              Serial.printf("%d\r\n", intArray[i]);
            }
            break;
        case '6':
          writeIntToEEPROM(0, testValue++);
          break;

        default:
            break;
        }
    }
}


void setup() {
  ::Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect.
  }
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);

  ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
  if (CrashReport) 
  {
      ::Serial.print(CrashReport);
      ::Serial.println();
      ::Serial.flush();
  }

  ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

  initEEPROM();
  
  // uint32_t value = WRITABLE_COUNT;
  // Serial.printf("Length = %d \r\n", writeCountLength);
  // for (int i = 0; i < writeCountLength; i++)
  // {
  //   writeCount[ reverseLength - i] = (value >> (8*i)) & 0xFF;
  //   EEPROM.write(reverseLength - i, writeCount[ reverseLength - i]);
  //   Serial.printf("TEMP Value : [%d] %6d = 0x%08x | 0x%04x \r\n", i, value >> (8*i), value >> (8*i), writeCount[ reverseLength - i]);
  // }
  
  // for (int i = 0; i < writeCountLength; i++)
  // {
  //   Serial.printf("[%d] %d |", i , writeCount[i]);
  // }
  // Serial.println();

  // uint8_t length = sizeof(uint32_t);
  // uint8_t length2 = sizeof(uint16_t);

  // Serial.printf("length : %d | %d\r\n", length, length2);

  //EEPROM.write();




  
  ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
  //::xTaskCreate(tickTock, "tickTock", 2048, nullptr, 1, nullptr);
  ::xTaskCreate(uartTask, "uartTask", 8196, nullptr, 1, nullptr);
  ::Serial.println("setup(): starting scheduler...");
  ::Serial.println("========Start Teensy Gripper========");
  ::Serial.flush(); // 단점 : UART 느림
  ::vTaskStartScheduler();

}

void loop() {
  // put your main code here, to run repeatedly:
}



void initEEPROM()
{
  bool initEEPROMFlag = false;
  for (int i = 0; i < 255; i++)
  {
    if(EEPROM.read(i) == 255)
    {
      initEEPROMFlag = true;
    }
    else
    {
      initEEPROMFlag = false;
    }
  }
  
  if(initEEPROMFlag == true)
  {
    Serial.printf("Clear EEPROM to 0\r\n");

    setAllEEPROM(0);
  }
}

void setAllEEPROM(int _value)
{
    for (int i = 0; i < 255; i++)
    {
      EEPROM.write(i, _value);
    }
}

uint32_t HexToInt(uint8_t* _hexArray)
{
  uint32_t result = 0;
  uint8_t length = sizeof(uint32_t);
  uint8_t reverseLength = length - 1;
  uint8_t* hexArray = _hexArray;
  

  Serial.printf("HexToInt Length : %d\r\n", length);


  for (int i = 0; i < length; i++)
  {
    //_hexArray[i] = EEPROM.read(i);
    result |= (hexArray[reverseLength - i] << 8*i);
  }

  _hexArray = hexArray;

  return result;
}

uint8_t* IntToHex(uint32_t _int)
{
  uint32_t value = _int;
  uint8_t* hexArray = nullptr;
  hexArray = new uint8_t[4];

  uint8_t length = sizeof(hexArray)/sizeof(uint8_t);
  uint8_t reverseLength = length - 1;

  for (int i = 0; i < length; i++)
  {
    hexArray[ reverseLength - i] = (value >> (8*i)) & 0xFF;
  }

  return hexArray;
}

//  4자리
uint8_t* writeEEPROMIntToHex(uint32_t _int, uint8_t _num, bool writable)
{
  uint32_t value = _int;
  uint8_t* hexArray = nullptr;
  hexArray = new uint8_t[4];

  uint8_t length = sizeof(hexArray)/sizeof(uint8_t);
  uint8_t reverseLength = length - 1;


  Serial.printf("IntToHex Length = %d \r\n", length);
  for (int i = 0; i < length; i++)
  {
    hexArray[ reverseLength - i] = (value >> (8*i)) & 0xFF;
    if(writable)
    {
      EEPROM.write(reverseLength - i + _num * 4, hexArray[ reverseLength - i]);
    }
    Serial.printf("TEMP Value : [%d] %6d = 0x%08x | 0x%04x \r\n", i, value >> (8*i), value >> (8*i), hexArray[ reverseLength - i]);
  }


  
  for (int i = 0; i < length; i++)
  {
    Serial.printf("Target Value : %d\r\n", hexArray[i]);
  }
  Serial.println();
  
  return hexArray;
}

bool writeIntToEEPROM(uint8_t _num, uint32_t _int)
{
  uint8_t* targetArray = IntToHex(_int);
  uint8_t length = sizeof(uint32_t)/ sizeof(uint8_t);
  uint8_t reverseLength = length - 1;

  Serial.printf("UINT32 length -> %d\r\n",length );

  for (int i = 0; i < length; i++)
  {
    EEPROM.write(reverseLength - i + _num * 4, targetArray[ reverseLength - i]);
  }

  return true;
}


void printEEPROM(uint8_t _start, uint8_t _end)
{
  for (size_t i = _start; i < _end; i++)
  {
    Serial.printf("[%d] %d \r\n", i, EEPROM.read(i));
  }

}