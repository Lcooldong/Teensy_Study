#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>
#include <DFRobot_TCS3430.h>
#include <PWMServo.h>

#include "neopixel.h"

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG
#define HWSERIAL Serial2  // RX2 : 7, TX2 : 8

#define SERVO_INITIAL_POS  0
#define SERVO_TARGET_POS  70

#define SERVO2_INITIAL_POS 10
#define SERVO2_TARGET_POS 40

#define COLOR_Y_MIN_VALUE 300
#define COLOR_Y_MAX_VALUE 700

#define HALL_MID_VALUE 2900
#define HALL_TARGET_VALUE 1310

#define HALL_FAR      0x00
#define HALL_NEARBY   0x04
#define HALL_ARRIVED  0x05
#define SERVO_CLOSED  0x06
#define SERVO_OPENED  0x00
#define COLOR_ON      0x07
#define COLOR_OFF     0x00
#define SERVO_RELEASE 0x00
#define SERVO_PUSH    0x08

const int Servo_Pin = 14;
const int Servo2_Pin = 15;

const int singNeopixel_Pin = 16;
const int ringNeopixel_Pin = 17;

const int SDA_Pin = 18;
const int SCL_Pin = 19;

const int hallSensor_Pin = 20;

const int blinkInterval =  300;
const int serialInterval = 1000;

typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t servoState;
  uint8_t hallState;
  uint8_t colorState;
  uint8_t buttonState;
  uint8_t etx;
}PACKET;

PACKET dataToSend = {0,};
PACKET buf;

int hallSensorValue = 0;
int hallCount = 0;
uint8_t gripperPos = 0;
uint8_t buttonPos = 0;

TaskHandle_t colorSensorHandle;
TaskHandle_t hallSensorHandle;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

MyNeopixel* myNeopixel = new MyNeopixel(12, ringNeopixel_Pin);
MyNeopixel* stateNeopixel = new MyNeopixel(1, singNeopixel_Pin);
DFRobot_TCS3430 tcs3430;
PWMServo gripperServo;
PWMServo buttonServo;



void initPacket(PACKET* _packet);
bool sendPacket(uint8_t* _data, size_t len);
void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond);

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
        ::Serial.println("TICK");
        HWSERIAL.println("HW : TICK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));

        ::Serial.println("TOCK");
        HWSERIAL.println("HW : TOCK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}


static void uartTask(void* ){
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){

      char text = Serial.read();
      if(text == 'u')
      {
        Serial.println("Servo2 UP");
        rotateServo(&buttonServo, SERVO2_INITIAL_POS, 5);        
        stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 255), 50, 1);
      }
      else if (text == 'd')
      {
        Serial.println("Servo2 DOWN");   
        rotateServo(&buttonServo, SERVO2_TARGET_POS, 2);
        stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 100), 50, 1);
        
      }
      else if (text == 'o')
      {
        rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
        Serial.println("Servo Open");
        stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 255, 0), 50, 1);
        
      }
      else if (text == 'c')
      {
        rotateServo(&gripperServo, SERVO_TARGET_POS, 5);
        Serial.println("Servo Close");
        stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 100), 50, 1);
        
      }
      else if (text == 'n')
      {
        vTaskResume(colorSensorHandle);
      }
      else if (text == 'f')
      {
        vTaskSuspend(colorSensorHandle);
        for (int i = 0; i < LED_COUNT; i++)
        {
            myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 2);
        }
      }
      else if (text == 'i')
      {
        sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
      }
      else if (text == 's')
      {
        vTaskResume(hallSensorHandle);
      }
      else if (text == 't')
      {
        vTaskSuspend(hallSensorHandle);
      }
      
      //vTaskDelay(pdMS_TO_TICKS(1));
      // vTaskDelayUntil(&xLastWakeTime, 1/portTICK_PERIOD_MS);
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
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
    
    ::vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void stopSensorTask(void*)
{ 
  
}

// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    
    while (!Serial)
    {
      delay(100);
    }
    
    ::Serial.println("========Start Teensy Gripper========");
    ::Serial.flush();
    HWSERIAL.begin(115200);
    Wire.begin();
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    gripperServo.attach(Servo_Pin);
    buttonServo.attach(Servo2_Pin);
    initPacket(&dataToSend);

    for (int i = 0; i < LED_COUNT; i++)
    {
        myNeopixel->pickOneLED(i, myNeopixel->strip->Color(0, 0, 0), 0, 2);
    }

    for (int i = 0; i < 10; i++)
    {
      if(tcs3430.begin())
      {
        break;
      }
      Serial.println("Please check that the IIC device is properly connected");
      delay(1000);
    }
    stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 0), 50, 1);
    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
    //::xTaskCreate(tickTock, "tickTock", 128, nullptr, 1, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 8192, nullptr, 2, nullptr);
    ::xTaskCreate(colorSensorTask, "ColorSensor", 8192, nullptr, 1, &colorSensorHandle);
    ::xTaskCreate(hallSensorTask, "hallSensorTask", 8192, nullptr, 1, &hallSensorHandle);
    //::xTaskCreate(stopSensorTask, "stopSensor", 8192, nullptr, 3, nullptr);
    ::Serial.println("setup(): starting scheduler...");
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
    
    ::delay(1'000);
  
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
    Serial.printf("0x%x \r\n", _data[i]);
    HWSERIAL.write(_data[i]);
  }
  
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
        //Serial.println("Button!");
        pos = buttonPos;
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
            //Serial.printf("Degree : %d\r\n", i);
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
            //Serial.printf("Degree : %d\r\n", i);
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
          buttonPos = pos;
        }
      }
}
