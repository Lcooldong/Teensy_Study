#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <HardwareSerial.h>
#include <Arduino.h>
#include <DFRobot_TCS3430.h>
#include "neopixel.h"
#include "myServo.h"
#include "MyLittleFS.h"
#include "config.h"

#define MYSERVO
#define MYLITTLEFS


PACKET dataToSend;

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1



int hallSensorValue = 0;
int hallCount = 0;

bool lastButtonValue = true; // Pull-Up -> 1
bool pressingButtonFlag = false;

uint64_t systemCount = 0; 

TaskHandle_t colorSensorHandle;
TaskHandle_t hallSensorHandle;
bool colorTestFlag = false;
bool hallTestFlag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;


void initPacket(PACKET* _packet);
bool sendPacket(uint8_t* _data, size_t len);
void toggleHallSensor(ToggleFlag _toggleFlag);
void toggleColorSensor(ToggleFlag _toggleFlag);
uint8_t* readString(uint32_t bufferSize);
uint8_t* ascii_to_hex( uint8_t* _string, uint8_t _size);
unsigned int ascii_to_hex2(const char* str, size_t size, uint8_t* hex);

#ifdef MYSERVO
MyServo* myServo = new MyServo();
#endif

#ifdef MYLITTLEFS
MyLittleFS* myLittleFS = new MyLittleFS();
#endif
MyNeopixel* myNeopixel = new MyNeopixel(12, ringNeopixel_Pin);
MyNeopixel* stateNeopixel = new MyNeopixel(1, singNeopixel_Pin);
DFRobot_TCS3430 tcs3430;

static void blink(void*) {
    while (true) {
        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
        ::vTaskDelay(pdMS_TO_TICKS(500));

        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void tickTock(void*) {
    while (true) {
        ::Serial.printf("Flow : %lu\r\n", systemCount++);
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}


String text = "";
uint8_t* receivedText = nullptr;
uint8_t* structArray = nullptr;

uint8_t hex[64] ={0,};


static void uartTask(void* ){
  //TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
      receivedText = readString(64);
      
      String charToString = (char*)receivedText;
      
      
      if(charToString.length() > 0)
      {
        int length = charToString.length() - 1;
        Serial.printf("String Length : %d\r\n", length );
        ascii_to_hex2(charToString.c_str(), length, hex);  // \n 포함되어있기에 - 1

        // for (int i = 0; i < length/2; i++)
        // {
        //   Serial.printf("[%d] 0x%02x\r\n", i,hex[i]);
        // }

        if(hex[0] == 0x02)
        {
          Serial.println("Start");
        }
        
        for (int i = 1; i < length/2 - 1; i++)
        {
          Serial.printf("[%d] 0x%02x\r\n", i, hex[i]);
        }  


        if(hex[length/2 - 1] == 0x03)
        {
          Serial.println("END");
        }
        
        sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
      }

      // structArray = new uint8_t[sizeof(dataToSend)];
      // structArray = ascii_to_hex(receivedText, charToString.length() - 1);
      // if(charToString.length() > 0)
      // {
      //   Serial.printf("String Length : %d\r\n", charToString.length() );

      //   for (uint8_t i = 0; i < sizeof(dataToSend); i++)
      //   {
      //     Serial.printf("0x%0x\r\n", structArray[i]);
      //   }

      //   Serial.println("-----------------------");
      // }
      
      
      // String hexString = "";
      
      // uint8_t v ;
      // if(charToString.length() > 0)
      // {
      //   for (uint8_t i = 0; i < sizeof(dataToSend) * 2; i++)
      //   {
          
      //     // hexString += String(receivedText[i]);
      //     // Serial.printf("%s\r\n", hexString);
      //     if(i % 2 == 0)
      //     {
      //       hexString = "";
      //       v = receivedText[i] << 4;
      //       Serial.printf("[000] - %d\r\n", v);
      //     }
      //     else if (i % 2 == 1)
      //     {
      //       v += receivedText[i];
            
      //       // Serial.printf("HEX : 0x%02x\r\n", v);
      //       Serial.printf("[111] - %d\r\n", v);
      //       // Serial.printf("[%d] [ARRAY %d ]- %s\r\n", i, i / 2, hexString); 
      //       // structArray[i/2] = hexString.toInt();    
      //     }         
      //     // Serial.printf("Data : %c \r\n", receivedText[i]);
      //   }
      // }
      
      
      // char ch = HWSERIAL.read();
      
      // if(ch == 0x02)
      // {
      //   int textCount = 0;
      //   while (true)
      //   {
      //     char packetByte = HWSERIAL.read();
      //     if(packetByte != 0xFF)  
      //     {
            
      //       Serial.printf("Input character[%d] : %c\r\n", textCount++, ch); // String -> Hex
      //     }
      //     else if(packetByte == 0x03)
      //     {
      //       textCount = 0;
      //       break;
      //     }
      //   }
      // }
      // else
      // {
        
      // }
      // vTaskDelay(pdMS_TO_TICKS(100));
      // String s = HWSERIAL.readStringUntil('\n');
      // if(s.length() > 0)
      // {
      //   Serial.printf("Received Text : %s\r\n",s);
      // }
      
      // while(HWSERIAL.read() != -1);
    // String context = HWSERIAL.readStringUntil('\n');
    // if(context.length() > 0)
    // {
    //   char* array = nullptr;
    //   context.toCharArray(array, context.length());
    //   Serial.printf("Received : %s\r\n" , context);
    //   for (uint32_t i = 0; i < context.length(); i++)
    //   {
    //     Serial.printf("Input Value : 0x%02X", array[i]);
    //   }
    //   Serial.println();
      
    // }
//       int packetCount = 0;
//       if(text == (char)0x02)
//       {
//         while (true)
//         {
//           packetCount++;
//           char packetData = HWSERIAL.parseInt();
//           Serial.printf("packet : %c\r\n", packetData);  
//           if(packetData == (char)0x03 || packetCount == sizeof(dataToSend)/sizeof(uint8_t))
//           {
//             Serial.println("End of Packet");
//             break;
//           }
//         } 
//       }
      
//       if(text == 'u')
//       {
// #ifdef MYSERVO
//         myServo->releaseServo();
// #endif
//         stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
// #ifdef MYLITTLEFS
//         myLittleFS->writeServoLog();
// #endif
//       }
//       else if (text == 'd')
//       {
// #ifdef MYSERVO
//         myServo->pushServo();      
// #endif
//         stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
// #ifdef MYLITTLEFS
//         myLittleFS->writeServoLog();
// #endif
//       }
//       else if (text == 'o')
//       {
// #ifdef MYSERVO
//         myServo->openServo();
// #endif
//         stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);
// #ifdef MYLITTLEFS
//         myLittleFS->writeServoLog();
// #endif
//       }
//       else if (text == 'c')
//       {
// #ifdef MYSERVO
//         myServo->closeServo();
// #endif
//         stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
// #ifdef MYLITTLEFS
//         myLittleFS->writeServoLog();
// #endif
//       }
//       else if (text == 'n')
//       {
//         Serial.println("========Color On========");
//         toggleColorSensor(ON);
//       }
//       else if (text == 'f')
//       {
//         Serial.println("========Color Off========");
//         toggleColorSensor(OFF);
        
//         for (int i = 0; i < LED_COUNT; i++)
//         {
//             myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 2);
//         }
//       }
//       else if (text == 'i')
//       {
//         Serial.println("Send Packet");
//         sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
//       }
//       else if (text == 's')
//       {
//         Serial.println("========Start reading hallSensor========");
//         toggleHallSensor(ON);
//       }
//       else if (text == 't')
//       {
//         Serial.println("========Stop reading hallSensor========");
//         toggleHallSensor(OFF);
//       }
      
      
      //vTaskDelay(pdMS_TO_TICKS(1));
      // vTaskDelayUntil(&xLastWakeTime, 1/portTICK_PERIOD_MS);
      //vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void uartTask2(void*)
{
  while (true)
  {
    char text = Serial.read();

    switch (text)
    {
    case 'l':
#ifdef MYLITTLEFS
      myLittleFS->listFiles();
#endif
      break;
    case 'd':
#ifdef MYLITTLEFS
      myLittleFS->dumpLog(myLittleFS->datalogFile);
#endif
      break;
    case 'e':
#ifdef MYLITTLEFS
        myLittleFS->eraseFiles();
#endif
      break;
    case '1':
#ifdef MYSERVO
      myServo->closeServo();
#endif
#ifdef MYLITTLEFS
      myLittleFS->writeServoLog();
#endif
      stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
      break;
    case '2':
#ifdef MYSERVO
      myServo->openServo();
#endif
#ifdef MYLITTLEFS
      myLittleFS->writeServoLog();
#endif
      stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);
      break;
    case '3':
#ifdef MYSERVO
      myServo->pushServo();
#endif
#ifdef MYLITTLEFS
      myLittleFS->writeServoLog();
#endif
      stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
      break;
    case '4':
#ifdef MYSERVO
      myServo->releaseServo();
#endif
#ifdef MYLITTLEFS
      myLittleFS->writeServoLog();
#endif
      stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
      break;
    case '5':
      if(!hallTestFlag)
      {
        toggleHallSensor(ON);
      }
      else
      {
        toggleHallSensor(OFF);
      }
      
      hallTestFlag = !hallTestFlag;
      break;
    case '6':
      if(!colorTestFlag)
      {
        toggleColorSensor(ON);
      }
      else
      {
        toggleColorSensor(OFF);
      }

      colorTestFlag = !colorTestFlag;
      break;
    case '\r': 
    case '\n':
    default:
      break;
    }

    while(Serial.read() != -1);
    ::vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static void colorSensorTask(void*)
{
  while (true)
  {
    uint16_t YData = tcs3430.getYData();

      // Send
    Serial.printf("Y Value : %d\r\n", YData);
    if(YData >= COLOR_Y_MAX_VALUE)
    {
      YData = COLOR_Y_MAX_VALUE;
    }
    else if(YData <= COLOR_Y_MIN_VALUE)
    {
      YData = COLOR_Y_MIN_VALUE;
    }
    int pwmValue = map(YData, COLOR_Y_MAX_VALUE, COLOR_Y_MIN_VALUE, 0, 255);
    int neopixelValue = map(pwmValue, 0, 255, 0, 250);
    for (int i = 0; i < LED_COUNT; i++)
    {
        myNeopixel->pickOneLED(i, myNeopixel->strip->Color(255, 255, 255), neopixelValue, 2);
    }
    
    
    ::vTaskDelay(pdMS_TO_TICKS(50));
  } 
}

static void hallSensorTask(void*)
{
  while (true)
  {
    
    hallSensorValue = analogRead(hallSensor_Pin);
    Serial.printf("Value : %d\r\n", hallSensorValue);

    if (hallSensorValue <= HALL_TARGET_VALUE)
    {
      hallCount++;
      if(hallCount > 10)
      {
        //Serial.println("Arrived at Target Height");
        dataToSend.hallState = HALL_ARRIVED;
      }   
    }
    else if(hallSensorValue <= HALL_MID_VALUE)
    {
      dataToSend.hallState = HALL_NEARBY;
    }
    else    
    {
      hallCount = 0;
      dataToSend.hallState = HALL_FAR;
    }
    
    ::vTaskDelay(pdMS_TO_TICKS(20));
  }
}

static void buttonTask(void*)
{
  while (true)
  {
    bool buttonValue = digitalRead(button_Pin);
    lastButtonValue = buttonValue;
    int openCloseCount = 0;

    //Serial.printf("Button Value : %d\r\n", buttonValue);
    //::vTaskDelay(pdMS_TO_TICKS(100));
    // Pull-Up
    if(buttonValue == 0)
    {

      // 놓을 때까지 지속 
      while(!digitalRead(button_Pin))
      {
        Serial.println("Start While");
        // 500ms 이상 눌렀을 때
        if(++openCloseCount >= 5){

          pressingButtonFlag = true;
          Serial.println("Button Pressing");
          if(dataToSend.servoState == SERVO_CLOSED)
          {
            //dataToSend.servoState = SERVO_OPENED;
            //Serial.println("Servo Open");
#ifdef MYSERVO
            myServo->openServo();
#endif
#ifdef MYLITTLEFS
            myLittleFS->writeServoLog();
#endif
            
            stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);
            
          }
          else if (dataToSend.servoState == SERVO_OPENED)
          {
            //dataToSend.servoState = SERVO_CLOSED;
            //Serial.println("Servo Close");
#ifdef MYSERVO
            myServo->closeServo(); 
#endif
#ifdef MYLITTLEFS
        myLittleFS->writeServoLog();
#endif
            stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
          }

          ::vTaskDelay(pdMS_TO_TICKS(500));
          break;       
        }
        else
        {
          pressingButtonFlag = false;
        }
        ::vTaskDelay(pdMS_TO_TICKS(100));
      }

      if(pressingButtonFlag == false)
      {
        Serial.println("Button Pressed");
        if(dataToSend.lockerState == SERVO_RELEASE)
        {
          // dataToSend.lockerState = SERVO_PUSH;
          // Serial.println("Servo Push");
#ifdef MYSERVO
          myServo->pushServo();
#endif
#ifdef MYLITTLEFS
        myLittleFS->writeServoLog();
#endif
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
          ::vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if (dataToSend.lockerState == SERVO_PUSH)
        {
          // dataToSend.lockerState = SERVO_RELEASE;
          // Serial.println("Servo Release");
#ifdef MYSERVO
          myServo->releaseServo();
#endif
#ifdef MYLITTLEFS
        myLittleFS->writeServoLog();
#endif
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
          ::vTaskDelay(pdMS_TO_TICKS(100));
        }
      }

      while (!digitalRead(button_Pin))
      {
        Serial.println("Release your button");
        ::vTaskDelay(pdMS_TO_TICKS(100));
      }


    }
  }
}



// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115200);
    HWSERIAL.begin(115200);
    
    Wire.begin();
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::pinMode(button_Pin, arduino::INPUT_PULLUP);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    
    initPacket(&dataToSend);

    //myLittleFS->initLittleFS();

    for (int i = 0; i < LED_COUNT; i++)
    {
        myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 10);
    }

    for (int i = 0; i < 5; i++)
    {
      if(tcs3430.begin())
      {
        Serial.println("Begin tcs3430 ColorSensor");
        break;
      }
      Serial.println("Please check that the IIC device is properly connected");
      delay(500);
    }
    stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 0), 20, 50);
    ::delay(100);
#ifdef MYLITTLEFS
    myLittleFS->initLittleFS();
#endif
    


    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
    // ::xTaskCreate(tickTock, "tickTock", 1024, nullptr, 1, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 8192, nullptr, 1, nullptr);
    ::xTaskCreate(uartTask2, "uartTask2", 8192, nullptr, 1, nullptr);
    ::xTaskCreate(colorSensorTask, "ColorSensor", 1024, nullptr, 2, &colorSensorHandle);
    ::xTaskCreate(hallSensorTask, "hallSensorTask", 1024, nullptr, 2, &hallSensorHandle);
    ::xTaskCreate(buttonTask, "ButtonTask", 1024, nullptr, 1, nullptr);

    //::xTaskCreate(stopSensorTask, "stopSensor", 8192, nullptr, 3, nullptr);
    ::Serial.println("setup(): starting scheduler...");
    ::Serial.println("========Start Teensy Gripper========");
    ::Serial.flush(); // 단점 : UART 느림

    // if( xReturned == pdPASS)
    // {
    //   Serial.printf("Create Input Task\r\n");
    //   ::Serial.flush();
    //   //vTaskDelete(xHandle);
    // }

    ::vTaskSuspend(colorSensorHandle);
    ::vTaskSuspend(hallSensorHandle);
    ::vTaskStartScheduler();
    
}







void loop() {}


void initPacket(PACKET* _packet)
{
  _packet->stx = 0x02;
  _packet->etx = 0x03;
}


bool sendPacket(uint8_t* _data, size_t len)
{

  for (size_t i = 0; i < len; i++)
  {
    //Serial.printf("0x%x \r\n", _data[i]);
    HWSERIAL.write(_data[i]);
  }
  //Serial.println("-----------");
  return true;
}

void toggleHallSensor(ToggleFlag _toggleFlag)
{ 
  if(_toggleFlag == ON)
  {
    vTaskResume(hallSensorHandle);

  }
  else
  {
    vTaskSuspend(hallSensorHandle);
    dataToSend.hallState = HALL_FAR;
  }
}

void toggleColorSensor(ToggleFlag _toggleFlag)
{
  if(_toggleFlag == ON)
  {
    vTaskResume(colorSensorHandle);
    dataToSend.colorState = COLOR_ON;
  }
  else
  {
    vTaskSuspend(colorSensorHandle);
    dataToSend.colorState = COLOR_OFF;
  }
}


uint8_t* readString(uint32_t _bufferSize)
{
  uint8_t* buffer = nullptr;

  buffer = new uint8_t[_bufferSize];
  int length = HWSERIAL.readBytesUntil('\r', buffer, _bufferSize); // \n전까지 읽음 hello\n 우면 5개 hello 만 읽음
  if(length > 0)
  {
    Serial.printf("length : %d\r\n", length);
    for (int i = 0; i < length; i++)
    {
      if(buffer[i] != '\0')
      {
        Serial.printf("%c", buffer[i]);
      }
      else
      {
        Serial.println("NULL");
      }
    }
  }

  return buffer;  // 이쪽 한번 더 봐야함
}

uint8_t* ascii_to_hex( uint8_t* _string, uint8_t _size)
{
  uint8_t* hexArray = nullptr;
  hexArray = new uint8_t[_size/2];
  uint8_t highBit, lowBit;

  for (uint8_t i = 0; i < _size; i++)
  {
    if(i % 2 == 0)
    {
      highBit = (_string[i] > '9') ? _string[i] - 'A' + 10 : _string[i] - '0';

    }
    else
    {
      lowBit = (_string[i + 1] > '9') ? _string[i + 1] - 'A' + 10 : _string[i + 1] - '0';
    }
    hexArray[_size/2] = (highBit << 4) | lowBit;
    
  }
  

  return hexArray;
}

unsigned int ascii_to_hex2(const char* str, size_t size, uint8_t* hex)
{
    unsigned int i, h, high, low;
    for (h = 0, i = 0; i < size; i += 2, ++h) {
        //9보다 큰 경우 : 알파벳 문자 'A' 이상인 문자로, 'A'를 빼고 10을 더함.
        //9이하인 경우 : 숫자 입력으로 '0'을 빼면 실제 값이 구해짐.
        high = (str[i] > '9') ? str[i] - 'A' + 10 : str[i] - '0';
        low = (str[i + 1] > '9') ? str[i + 1] - 'A' + 10 : str[i + 1] - '0';
        //high 4비트, low 4비트이므로, 1바이트를 만들어주기 위해 high를 왼쪽으로 4비트 shift
        //이후 OR(|)연산으로 합
        hex[h] = (high << 4) | low;
    }
    return h;
}