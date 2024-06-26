#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include "Config.h"
#include "neopixel.h"

MyNeopixel* myNeopixel = new MyNeopixel(1, 23);

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
        ::vTaskDelay(pdMS_TO_TICKS(1'000));

        ::Serial.println("TOCK");
        ::vTaskDelay(pdMS_TO_TICKS(1'000));
    }
}

static void uartTask(void*){
  while (true)
  {
    ::Serial.printf("Value %d\r\n", testValue++);

    ::vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 1);
    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    ::xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 2, nullptr);
    ::xTaskCreate(uartTask, "uartTask", 4096, nullptr, 2, nullptr);

    ::Serial.println("setup(): starting scheduler...");
    ::Serial.flush();

    ::vTaskStartScheduler();
}

void loop() {}