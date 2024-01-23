#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>
#include <DFRobot_TCS3430.h>
#include <PWMServo.h>
#include <LittleFS.h>

#include "neopixel.h"

#define OLD_PINOUT
//#define NEW_PINOUT

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG
#define HWSERIAL Serial2  // RX2 : 7, TX2 : 8

#define SERVO_INITIAL_POS  0
#define SERVO_TARGET_POS  70

#define SERVO2_INITIAL_POS 10
#define SERVO2_TARGET_POS 40

#define COLOR_Y_MIN_VALUE 250
#define COLOR_Y_MAX_VALUE 550

#define HALL_MID_VALUE 600
#define HALL_TARGET_VALUE 300

#define HALL_FAR      0x00
#define HALL_NEARBY   0x04
#define HALL_ARRIVED  0x05
#define SERVO_CLOSED  0x06
#define SERVO_OPENED  0x00
#define COLOR_ON      0x07
#define COLOR_OFF     0x00
#define SERVO_RELEASE 0x00
#define SERVO_PUSH    0x08

#define PROG_FLASH_SIZE 1024 * 900

#ifdef OLD_PINOUT
const int Servo_Pin = 14;
const int Servo2_Pin = 15;

const int singNeopixel_Pin = 16;
const int ringNeopixel_Pin = 17;

const int SDA_Pin = 18;
const int SCL_Pin = 19;

const int hallSensor_Pin = 20;
const int button_Pin = 21;
#endif

#ifdef NEW_PINOUT
const int Servo_Pin = 16;
const int Servo2_Pin = 17;

const int singNeopixel_Pin = 18;
const int ringNeopixel_Pin = 19;

const int SDA_Pin = 20;
const int SCL_Pin = 21;

const int hallSensor_Pin = 22;
const int button_Pin = 23;
#endif

const int blinkInterval =  300;
const int serialInterval = 1000;

typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t servoState;
  uint8_t hallState;
  uint8_t colorState;
  uint8_t lockerState;
  uint8_t etx;
}PACKET;

PACKET dataToSend;
PACKET buf;

int hallSensorValue = 0;
int hallCount = 0;
uint8_t gripperPos = 0;
uint8_t lockerPos = 0;
bool lastButtonValue = true; // Pull-Up -> 1
bool pressingButtonFlag = false;

uint64_t systemCount = 0;

TaskHandle_t colorSensorHandle;
TaskHandle_t hallSensorHandle;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

MyNeopixel* myNeopixel = new MyNeopixel(12, ringNeopixel_Pin);
MyNeopixel* stateNeopixel = new MyNeopixel(1, singNeopixel_Pin);
DFRobot_TCS3430 tcs3430;
PWMServo gripperServo;
PWMServo lockerServo;

LittleFS_Program myfs;
File dataFile;
int record_count = 0;
const char* datalogFile = "datalog.txt";

void initPacket(PACKET* _packet);
bool sendPacket(uint8_t* _data, size_t len);
void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond);

void openServo();
void closeServo();
void pushServo();
void releaseServo();


void eraseFiles();
void initLittleFS();
void listFiles();
void printDirectory(FS &fs);
void printDirectory(File dir, int numSpaces);
void printSpaces(int num);
void dumpLog(const char* _file);
uint8_t writeServoLog();
void loadLogData();

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
        //::Serial.println("TICK");
        //HWSERIAL.println("HW : TICK");
        //::vTaskDelay(pdMS_TO_TICKS(serialInterval));

        //::Serial.println("TOCK");
        //HWSERIAL.println("HW : TOCK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}


static void uartTask(void* ){
  //TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){

      char text = HWSERIAL.read();
      

      if(text == 'u')
      {
        releaseServo();
        // Serial.println("Servo2 UP");
        // rotateServo(&lockerServo, SERVO2_INITIAL_POS, 5);
        // dataToSend.lockerState = SERVO_RELEASE;
        // sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
        // stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
      }
      else if (text == 'd')
      {
        pushServo();

        // Serial.println("Servo2 DOWN");   
        // rotateServo(&lockerServo, SERVO2_TARGET_POS, 2);
        // dataToSend.lockerState = SERVO_PUSH;
        // sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
        // stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
        
      }
      else if (text == 'o')
      {
        openServo();

        // rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
        // Serial.println("========Servo Open========");
        // dataToSend.servoState = SERVO_OPENED;
        // sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
        // stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);
        
      }
      else if (text == 'c')
      {
        closeServo();

        // rotateServo(&gripperServo, SERVO_TARGET_POS, 5);
        // Serial.println("========Servo Close========");
        // dataToSend.servoState = SERVO_CLOSED;
        // sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
        // stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
        
      }
      else if (text == 'n')
      {
        Serial.println("========Color On========");
        vTaskResume(colorSensorHandle);
        dataToSend.colorState = COLOR_ON;
      }
      else if (text == 'f')
      {
        vTaskSuspend(colorSensorHandle);
        Serial.println("========Color Off========");
        dataToSend.colorState = COLOR_OFF;
        for (int i = 0; i < LED_COUNT; i++)
        {
            myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 2);
        }
      }
      else if (text == 'i')
      {
        Serial.println("Send Packet");
        sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
      }
      else if (text == 's')
      {
        Serial.println("========Start reading hallSensor========");
        vTaskResume(hallSensorHandle);
      }
      else if (text == 't')
      {
        Serial.println("========Stop reading hallSensor========");
        vTaskSuspend(hallSensorHandle);
        dataToSend.hallState = HALL_FAR;
      }
      
      
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
      listFiles();
      break;
    case 'd':
      dumpLog("datalog.txt");
      break;
    case 'e':
      eraseFiles();
      break;
    case '1':
      closeServo();
      break;
    case '2':
      openServo();
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
            openServo();
            
          }
          else if (dataToSend.servoState == SERVO_OPENED)
          {
            //dataToSend.servoState = SERVO_CLOSED;
            //Serial.println("Servo Close");
            closeServo(); 
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
          pushServo();
          ::vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if (dataToSend.lockerState == SERVO_PUSH)
        {
          // dataToSend.lockerState = SERVO_RELEASE;
          // Serial.println("Servo Release");
          releaseServo();
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
    gripperServo.attach(Servo_Pin);
    lockerServo.attach(Servo2_Pin);
    initPacket(&dataToSend);

    // initLittleFS();

    for (int i = 0; i < LED_COUNT; i++)
    {
        myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 10);
    }

    for (int i = 0; i < 10; i++)
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

    initLittleFS();

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
    ::xTaskCreate(tickTock, "tickTock", 2048, nullptr, 1, nullptr);
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

void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond)
{     
      int pos;
      //delay(5);

      if(_servo == &gripperServo)
      {
        //Serial.println("Gripper!");
        pos = gripperPos;
      }
      else
      {
        //Serial.println("Locker!");
        pos = lockerPos;
      }

      if (pos != targetPos)
      {
        //Serial.print("Servo Rotate Start\r\n");

        if(pos < targetPos)
        {
          
          for (int i = 0; i <= targetPos; i++)
          {
            //gripperServo.write(i);
            _servo->write(i);
            pos = i;
            Serial.printf("Up Degree : %d\r\n", i);
            //delay(millisecond);
            ::vTaskDelay(pdMS_TO_TICKS(millisecond));
          }  
        }
        else if (pos > targetPos)
        {
          for (int i = pos; i >= targetPos; i--)
          {
            //gripperServo.write(i);
            _servo->write(i);
            pos = i;
            Serial.printf("Down Degree : %d\r\n", i);
            //delay(millisecond);
            ::vTaskDelay(pdMS_TO_TICKS(millisecond));
          }
        }
        //digitalWrite(Servo_Pin, arduino::LOW);      // 끄기
        //Serial.printf("Servo Rotated\r\n");
        //_servo->detach();
        //delay(5);
        if(_servo == &gripperServo)
        {
          //Serial.println("Gripper!");
          gripperPos = pos;
        }
        else
        {
          //Serial.println("Button!");
          lockerPos = pos;
        }

      }
}


void openServo()
{
  rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
  Serial.println("========Servo Open========");
  dataToSend.servoState = SERVO_OPENED;
  //sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
  stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 10, 1);
  gripperPos = writeServoLog();
}

void closeServo()
{
  rotateServo(&gripperServo, SERVO_TARGET_POS, 5);
  Serial.println("========Servo Close========");
  dataToSend.servoState = SERVO_CLOSED;
  //sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
  stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 10, 1);
  gripperPos = writeServoLog();
}

void pushServo()
{
  Serial.println("Servo2 DOWN");   
  rotateServo(&lockerServo, SERVO2_TARGET_POS, 2);
  dataToSend.lockerState = SERVO_PUSH;
  //sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
  stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 10, 1);
}

void releaseServo()
{
  Serial.println("Servo2 UP");
  rotateServo(&lockerServo, SERVO2_INITIAL_POS, 5);
  dataToSend.lockerState = SERVO_RELEASE;
  //sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
  stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 10, 1);
}

void initLittleFS()
{
  if (!myfs.begin(PROG_FLASH_SIZE)) {
    Serial.printf("Error starting %s\n", "PROGRAM FLASH DISK");
    while (1) {
      Serial.printf("Failed to Mount \r\n");
      delay(500);
    }
  }
  Serial.println("LittleFS initialized.");
  listFiles();

  loadLogData();
}

void eraseFiles()
{
  myfs.quickFormat();  // performs a quick format of the created di
  Serial.println("\nFiles erased !");
}

void listFiles()
{
  Serial.print("\nSpace Used = ");
  Serial.println(myfs.usedSize());
  Serial.print("Filesystem Size = ");
  Serial.println(myfs.totalSize());

  printDirectory(myfs);
}

void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open("/"), 0);
  Serial.println();
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile(); // 다음 파일 열기
     if (! entry) {
       //Serial.println("** no more files **");
       break;   // 더 이상 파일이 없으면 나오기
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());  // 파일 이름
     if (entry.isDirectory()) {   // 디렉토리이면 / 추가
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));  
       Serial.print("  ");
       Serial.println(entry.size(), arduino::DEC);   // 파일 크기
     }
     entry.close(); // 파일 종료
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void dumpLog(const char* _file)
{
  Serial.println("\nDumping Log!!!");
  // open the file.
  dataFile = myfs.open(_file);

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}

uint8_t writeServoLog() 
{
  int value = gripperPos;

  myfs.remove(datalogFile);

  dataFile = myfs.open(datalogFile, FILE_WRITE);
  
  

  String dataString = "";
  dataString = "Gripper:" + String(gripperPos); 

  if(dataFile)
  {
    dataFile.println(dataString);
  }
  else
  {
    Serial.println("error opening datalog.txt");
  }

  dataFile.close();
  delay(50);

  return value;
}

void loadLogData()
{
  dataFile = myfs.open(datalogFile);

  String readData;

  if(dataFile)
  {
    while(dataFile.available())
    {
      readData = dataFile.readStringUntil('\n');
      if(readData.startsWith("Gripper:"))
      {
        Serial.print(readData);
        gripperPos = readData.substring(readData.indexOf(':')+1, readData.length()).toInt();
        if(gripperPos == 70)
        {
          dataToSend.servoState = SERVO_CLOSED;
        }
        else if (gripperPos == 0)
        {
          dataToSend.servoState = SERVO_OPENED;
        }
      }
    }
    dataFile.close();
  }
  else
  {
    Serial.println("error opening datalog.txt");
  }
}