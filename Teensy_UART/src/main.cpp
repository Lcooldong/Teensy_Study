#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"
//#include <SoftwareSerial.h>   // Not Working in my case
#include <HardwareSerial.h>

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG
#define HWSERIAL Serial2

const int Servo_Pin = 15;
const int SoftRx_Pin = 5;
const int SoftTx_Pin = 6;
int hallSensorValue = 0;

const int blinkInterval =  300;
const int serialInterval = 1000;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

//SoftwareSerial mySofSerial(SoftRx_Pin, SoftTx_Pin);


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
    // if(Serial.available())
    // {
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


  // }
}



// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    HWSERIAL.begin(115200);
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

    
    ::xTaskCreate(task1, "task1", 128, nullptr, 1, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 2, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 8192, nullptr, 3, nullptr);
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

