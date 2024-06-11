#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>
#include "neopixel.h"

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG
#define HWSERIAL Serial2
#define PINCOUNT 13

const int SoftRx_Pin = 5;
const int SoftTx_Pin = 6;
int hallSensorValue = 0;

const int blinkInterval =  300;
const int serialInterval = 1000;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

MyNeopixel* led[PINCOUNT];

// MyNeopixel* led1 = new MyNeopixel(10, 0);
// MyNeopixel* led2 = new MyNeopixel(10, 1);
// MyNeopixel* led3 = new MyNeopixel(10, 2);
// MyNeopixel* led4 = new MyNeopixel(10, 3);
// MyNeopixel* led5 = new MyNeopixel(10, 4);
// MyNeopixel* led6 = new MyNeopixel(10, 5);
// MyNeopixel* led7 = new MyNeopixel(10, 6);
// MyNeopixel* led8 = new MyNeopixel(10, 7);
// MyNeopixel* led9 = new MyNeopixel(10, 8);
// MyNeopixel* led10 = new MyNeopixel(10, 9);
// MyNeopixel* led11 = new MyNeopixel(10, 10);
// MyNeopixel* led12 = new MyNeopixel(10, 11);
// MyNeopixel* led13 = new MyNeopixel(10, 12);

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
      if(text == 'a')
      {
        Serial.println("Press a");
        HWSERIAL.println("HW : Press a");
        for (int i = 0; i < LED_COUNT; i++)
        {
            led[i]->pickOneLED(i, led[i]->strip->Color(255, 255, 255), 255, 1);
        }
        
      }
      else if (text == 'b')
      {
        Serial.println("Press b");
        HWSERIAL.println("HW : Press b");
        for (int i = 0; i < LED_COUNT; i++)
        {
            led[i]->pickOneLED(i, led[i]->strip->Color(0, 0, 0), 0, 1);
        }
      }
      
      //vTaskDelay(pdMS_TO_TICKS(1));
      // vTaskDelayUntil(&xLastWakeTime, 1/portTICK_PERIOD_MS);
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void ledTask(void *){

    while (true)
    {
        for (int i = 0; i < PINCOUNT; i++)
        {
            led[i]->pickOneLED(0, led[i]->strip->Color(255, 100, 255), 255, 1);
            vTaskDelay(pdMS_TO_TICKS(10));
            led[i]->pickOneLED(0, led[i]->strip->Color(0, 0, 0), 0, 1);
            vTaskDelay(pdMS_TO_TICKS(10));
            // for (int j=0; j < 256; j++) 
            // {     // cycle all 256 colors in the wheel
            //     for (int q=0; q < 3; q++) 
            //     {
            //         for (uint16_t i=0; i < led[i]->strip->numPixels(); i=i+3) 
            //         {
            //             led[i]->strip->setPixelColor(i+q, led[i]->Wheel( (i+j) % 255));    //turn every third pixel on
            //         }
            //         led[i]->strip->show();

            //         vTaskDelay(pdMS_TO_TICKS(1));

            //         for (uint16_t i=0; i < led[i]->strip->numPixels(); i=i+3) 
            //         {
            //             led[i]->strip->setPixelColor(i+q, 0);        //turn every third pixel off
            //         }
            //     }
            // }
            
        }
        
    }
    

}



// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    HWSERIAL.begin(115200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);

    for (int i = 0; i < PINCOUNT; i++)
    {
        led[i] = new MyNeopixel(LED_COUNT, i);
        led[i]->InitNeopixel();
    }
    
    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(task1, "task1", 128, nullptr, 1, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 1, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 8192, nullptr, 2, nullptr);
    ::xTaskCreate(ledTask, "LED_Task", 8192, nullptr, 1, nullptr);
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

