#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG

const int hallSensor_Pin = 14;
int hallSensorValue = 0;

const int blinkInterval =  300;
const int serialInterval = 1000;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

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
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));

        ::Serial.println("TOCK");
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}

static void analogInputTask(void* ){
  while(true){
    hallSensorValue = analogRead(hallSensor_Pin);
#ifdef DEBUG
    ::Serial.printf("Hall : %d\r\n", hallSensorValue);
    ::Serial.flush();
#endif
    if(hallSensorValue < 350)
    {
      ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    }
    else
    {
      ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
    }

    ::vTaskDelay(pdMS_TO_TICKS(100));


  }
}



// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);

    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
//    ::xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 1, nullptr);
    ::xTaskCreate(analogInputTask, "analogInputTask", 8192, nullptr, 3, nullptr);
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

