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
char* readString(uint32_t bufferSize);
uint8_t* ascii_to_hex_simple( uint8_t* _string, uint8_t _size);
unsigned int ascii_to_hex(const char* str, size_t size, uint8_t* hex);
uint8_t caculateCheckSum(uint8_t* _hex, uint8_t _size);

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




uint8_t* structArray = nullptr;
char* receivedText = nullptr;
uint8_t hex[256] ={0,};


static void uartTask(void* ){
  //TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
      receivedText = readString(256);
      
      if(String(receivedText).length() > 0)
      {
        int length = String(receivedText).length() - 1; // \n 포함되어있기에 - 1    
        int endOfText = length/2 - 1;           // 처음 빼기 -1 (stx)
        Serial.printf("String Length : %d\r\n", length );
        ascii_to_hex(receivedText, length, hex);

        // for (int i = 0; i < length/2; i++)
        // {
        //   Serial.printf("[%d] 0x%02x\r\n", i,hex[i]);
        // }

        if(hex[0] == 0x02)
        {
          Serial.println("--Start--");
        }
        
        for (int i = 1; i < endOfText; i++) // 1 ~ 5
        {
          Serial.printf("[%d] 0x%02x\r\n", i, hex[i]);
        }


        switch (hex[1])
        {
        case RESPONSE_SERVO_OPEN:
          myServo->openServo();
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);

          break;

        case RESPONSE_SERVO_CLOSE:
          myServo->closeServo();
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
          break;

        case RESPONSE_LOCKER_RELEASE:
          myServo->releaseServo();
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
          break;

        case RESPONSE_LOCKER_PUSH:
          myServo->pushServo();
          stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
          break;

        case RESPONSE_HALL_ON:
          toggleHallSensor(ON);
          break;

        case RESPONSE_HALL_OFF:
          toggleHallSensor(OFF);
          break;

        case RESPONSE_COLOER_ON:
          toggleColorSensor(ON);
          break;

        case RESPONSE_COLOER_OFF:
          toggleColorSensor(OFF);
          break;

        default:
          break;
        }

  
        dataToSend.checksum = caculateCheckSum(hex, endOfText);


        if(hex[endOfText] == 0x03)
        {
          Serial.println("-- END --");
        }

        if(dataToSend.checksum == 0xAA)
        {
          
          sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
          Serial.println("Send Return Packet");
        }
      }

      // vTaskDelay(pdMS_TO_TICKS(1));
      
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
        myNeopixel->pickOneLED(i, myNeopixel->strip->Color(255, 255, 255), neopixelValue, 1);
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
    
    ::vTaskDelay(pdMS_TO_TICKS(10));
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
    Serial.setTimeout(50);
    Serial.begin(115200);
    HWSERIAL.setTimeout(50);
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
    // ::xTaskCreate(uartTask2, "uartTask2", 8192, nullptr, 1, nullptr);
    ::xTaskCreate(colorSensorTask, "ColorSensor", 1024, nullptr, 2, &colorSensorHandle);
    ::xTaskCreate(hallSensorTask, "hallSensorTask", 1024, nullptr, 2, &hallSensorHandle);
    ::xTaskCreate(buttonTask, "ButtonTask", 512, nullptr, 1, nullptr);

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
  Serial.println("-----------");
  return true;
}

void toggleHallSensor(ToggleFlag _toggleFlag)
{ 
  if(_toggleFlag == ON)
  {
    vTaskResume(hallSensorHandle);
    Serial.println("Start HallSensor");
    // ::vTaskDelay(pdMS_TO_TICKS(10));
    
  }
  else
  {
    Serial.println("Stop HallSensor");

    vTaskSuspend(hallSensorHandle);
    dataToSend.hallState = HALL_FAR;
    // ::vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void toggleColorSensor(ToggleFlag _toggleFlag)
{
  if(_toggleFlag == ON)
  {
    dataToSend.colorState = COLOR_ON;
    vTaskResume(colorSensorHandle);    
    // ::vTaskDelay(pdMS_TO_TICKS(10));
  }
  else
  {
    dataToSend.colorState = COLOR_OFF;
    vTaskSuspend(colorSensorHandle); 
    // ::vTaskDelay(pdMS_TO_TICKS(10));
  }
}


char* readString(uint32_t _bufferSize)
{
  char* buffer = nullptr;

  buffer = new char[_bufferSize];
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

uint8_t* ascii_to_hex_simple( uint8_t* _string, uint8_t _size)
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

unsigned int ascii_to_hex(const char* str, size_t size, uint8_t* hex)
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

uint8_t caculateCheckSum(uint8_t* _hex, uint8_t _size)
{
  uint8_t resultValue = 0;
  // for (int i = 1; i < _size; i++)
  // {
  //   resultValue += _hex[i];
  // }
  resultValue  = _hex[_size - 1] << 1; 

  Serial.printf("CHECKSUM : 0x%02X\r\n", resultValue);
  
  return resultValue;
}

