#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>
#include <PWMServo.h>

#include "neopixel.h"

const int serialInterval = 1000;
uint64_t systemCount = 0;

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

void setup() {
  ::Serial.begin(115200);
  ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
  if (CrashReport) 
  {
      ::Serial.print(CrashReport);
      ::Serial.println();
      ::Serial.flush();
  }

  ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

  
  ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
  ::xTaskCreate(tickTock, "tickTock", 2048, nullptr, 1, nullptr);

  ::Serial.println("setup(): starting scheduler...");
  ::Serial.println("========Start Teensy Gripper========");
  ::Serial.flush(); // 단점 : UART 느림
  ::vTaskStartScheduler();
}

void loop() {}

