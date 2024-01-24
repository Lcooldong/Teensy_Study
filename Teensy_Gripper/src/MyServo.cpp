#include "MyServo.h"
#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

//extern PACKET dataToSend;
//extern MyLittleFS* myLittleFS;

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


void MyServo::openServo()
{
  rotateServo(&gripperServo, SERVO_INITIAL_POS, 5);
  Serial.println("========Servo Open========");
  this->dataToSendServo->servoState = SERVO_OPENED;
  //sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
}

void MyServo::closeServo()
{
  rotateServo(&gripperServo, SERVO_TARGET_POS, 5);
  Serial.println("========Servo Close========");
  this->dataToSendServo->servoState = SERVO_CLOSED; 
}

void MyServo::pushServo()
{
  Serial.println("Servo2 DOWN");   
  rotateServo(&lockerServo, SERVO2_TARGET_POS, 2);
  this->dataToSendServo->lockerState = SERVO_PUSH; 
}

void MyServo::releaseServo()
{
  Serial.println("Servo2 UP");
  rotateServo(&lockerServo, SERVO2_INITIAL_POS, 5);
  this->dataToSendServo->lockerState = SERVO_RELEASE;
}