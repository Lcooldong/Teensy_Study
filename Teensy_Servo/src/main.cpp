#include <Arduino.h>

// #include <PWMServo.h>

// PWMServo myservo;  // create servo object to control a servo

// int pos = 0;    // variable to store the servo position

// void setup() {
//   myservo.attach(15);  // attaches the servo on pin 9 to the servo object
//   //myservo.attach(SERVO_PIN_A, 1000, 2000); // some motors need min/max setting
// }


// void loop() {
//   for(pos = 0; pos < 180; pos += 1) { // goes from 0 degrees to 180 degrees, 1 degree steps
//     myservo.write(pos);              // tell servo to go to position in variable 'pos'
//     delay(15);                       // waits 15ms for the servo to reach the position
//   }
//   for(pos = 180; pos>=1; pos-=1) {   // goes from 180 degrees to 0 degrees
//     myservo.write(pos);              // tell servo to go to position in variable 'pos'
//     delay(15);                       // waits 15ms for the servo to reach the position
//   }
// }



#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <PWMServo.h>

//#undef configUSE_TIME_SLICING
//#define configUSE_TIME_SLICING 1

#define DEBUG
#define AUTO_SERVO
//#define SERIAL_SERVO

const int ServoPotentionMeter_Pin = 14;
const int Servo_Pin = 15;
int hallSensorValue = 0;

const int blinkInterval =  300;
const int serialInterval = 5000;
bool flag = false;

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

PWMServo myServo;


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
      //Serial.printf("Current Angle : %d  \r\n", myServo.read());
      ::Serial.println("TICK");
      ::vTaskDelay(pdMS_TO_TICKS(serialInterval));

      ::Serial.println("TOCK");
      ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}

int pos = 0;
static void servoTask(void* ){
  while(true){

#ifdef SERIAL_SERVO
    char text = Serial.read();

    if(text == ',')
    {
      myServo.write(--pos);
      Serial.printf("POS : %d\r\n", pos);
      
    }
    else if(text == '.')
    {
      myServo.write(++pos);
      Serial.printf("POS : %d\r\n", pos);
    }
  
#endif

#ifdef AUTO_SERVO
    for(pos = 0; pos < 180; pos += 1) { // goes from 0 degrees to 180 degrees, 1 degree steps
      myServo.write(pos);              // tell servo to go to position in variable 'pos'
      Serial.printf("POS : %d\r\n", pos);
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    vTaskDelay(500);

    for(pos = 180; pos>=1; pos-=1) {   // goes from 180 degrees to 0 degrees
      myServo.write(pos);              // tell servo to go to position in variable 'pos'
      Serial.printf("POS : %d\r\n", pos);
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    
    vTaskDelay(500);
#endif

  }
}



// Setup
FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    myServo.attach(Servo_Pin);

    ::delay(1'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    
    ::xTaskCreate(task1, "task1", 128, nullptr, 3, nullptr);
    ::xTaskCreate(task2, "task2", 128, nullptr, 1, nullptr);
    ::xTaskCreate(servoTask, "servoTask", 8192, nullptr, 2, nullptr);
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

