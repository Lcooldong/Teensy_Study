#include "myServo.h"
#include "MyLittleFS.h"
#include "neopixel.h"
#include "arduino_freertos.h"

extern PACKET dataToSend;
extern MyLittleFS* myLittleFS;
extern MyNeopixel* stateNeopixel;

MyServo::MyServo(/* args */)
{
  gripperPos = SERVO_INITIAL_POS;
  lockerPos = SERVO2_INITIAL_POS;
}

MyServo::~MyServo()
{
}


void MyServo::initServo()
{
    gripperServo.attach(Servo_Pin);
    lockerServo.attach(Servo2_Pin);

}

void MyServo::rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond)
{     
      int pos;
      //delay(5);

      if(_servo == &(this->gripperServo))
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
          
          for (int i = pos; i <= targetPos; i++)
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


void MyServo::openServo()
{
  //gripperServo.attach(Servo_Pin);
  vTaskDelay(pdMS_TO_TICKS(100));

  rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
  Serial.println("========Servo Open========");
  dataToSend.servoState = SERVO_OPENED;
  myLittleFS->writeServoLog();

  //gripperServo.detach();
}

void MyServo::openServo(bool hallRangeOn)
{
  int timeOutCount = 0;
  while (timeOutCount < 100)
  {
    if(dataToSend.hallState == HALL_ARRIVED)
    {
      rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
      Serial.println("========Servo Open========");
      dataToSend.servoState = SERVO_OPENED;
      myLittleFS->writeServoLog(); 
      
      break;
    }
    else
    {
      timeOutCount++;
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  Serial.println("HallSensor Error");
  for (int i = 0; i < 10; i++)
  {
    stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(0, 0, 0), 0, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));    
    stateNeopixel->pickOneLED(0, stateNeopixel->strip->Color(255, 0, 0), 50, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void MyServo::closeServo()
{
  //gripperServo.attach(Servo_Pin);
  vTaskDelay(pdMS_TO_TICKS(100));

  rotateServo(&gripperServo, SERVO_TARGET_POS, 5);
  Serial.println("========Servo Close========");
  dataToSend.servoState = SERVO_CLOSED;
  myLittleFS->writeServoLog();

  //gripperServo.detach();
}

void MyServo::pushServo()
{
  Serial.println("Servo2 DOWN");   
  rotateServo(&lockerServo, SERVO2_TARGET_POS, 2);
  dataToSend.lockerState = SERVO_PUSH;
  myLittleFS->writeServoLog();
}

void MyServo::releaseServo()
{
  Serial.println("Servo2 UP");
  rotateServo(&lockerServo, SERVO2_INITIAL_POS, 5);
  dataToSend.lockerState = SERVO_RELEASE;
  myLittleFS->writeServoLog();
}