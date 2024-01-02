#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"
//#include <SoftwareSerial.h>   // Not Working in my case
#include <HardwareSerial.h>
#include <Wire.h>
#include "DFRobot_TCS3430.h"


//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1


#define DEBUG
#define HWSERIAL Serial2

const int Servo_Pin = 15;
const int SDA_Pin = 18;
const int SCL_Pin = 19;



int hallSensorValue = 0;

const int blinkInterval =  300;
const int serialInterval = 1000;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

DFRobot_TCS3430 tcs3430;


static void task1(void*) {
    while (true) {
        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
        ::vTaskDelay(pdMS_TO_TICKS(500));

        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void task2(void*) {
    while (true) {
        ::Serial.println("TICK");
        HWSERIAL.println("HW : TICK");
  //      mySofSerial.println("SW : TICK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));

        ::Serial.println("TOCK");
        HWSERIAL.println("HW : TOCK");
//        mySofSerial.println("SW : TOCK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}


static void uartTask(void* ){
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
      char text = Serial.read();
      if(text == 'a')
      {
        Serial.println("Press a");
        HWSERIAL.println("HW : Press a");
//        mySofSerial.println("SW : Press a");
      }
      else if (text == 'b')
      {
        Serial.println("Press b");
        HWSERIAL.println("HW : Press b");
//        mySofSerial.println("SW : Press b");
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
    Serial.printf("Y Value : %d\r\n", YData);
    delay(100);
  } 
}




// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    HWSERIAL.begin(115200);
    Wire.begin();


    for (int i = 0; i < 10; i++)
    {
      if(tcs3430.begin())
      {
        break;
      }
      Serial.println("Please check that the IIC device is properly connected");
      delay(1000);
    }
    
    // while(!tcs3430.begin())
    // {
    //   Serial.println("Please check that the IIC device is properly connected");
    //   delay(1000);
    // }
  //  mySofSerial.begin(115200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);

    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 2, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 8192, nullptr, 3, nullptr);
    ::xTaskCreate(colorSensorTask, "ColorSensor", 8192, nullptr, 2, nullptr);
    ::Serial.println("setup(): starting scheduler...");
    ::Serial.flush(); // 단점 : UART 느림

    // if( xReturned == pdPASS)
    // {
    //   Serial.printf("Create Input Task\r\n");
    //   ::Serial.flush();
    //   //vTaskDelete(xHandle);
    // }

    ::vTaskStartScheduler();
}






void loop() {}

