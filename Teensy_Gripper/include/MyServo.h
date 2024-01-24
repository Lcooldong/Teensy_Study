#ifndef __MYSERVO_H__
#define __MYSERVO_H__


#include <PWMServo.h>
#include "config.h"








class MyServo
{
private:
    PWMServo gripperServo;
    PWMServo lockerServo;

    // const int Servo_Pin = 16;
    // const int Servo2_Pin = 17;
    

public:
    MyServo(PACKET* _packet);
    ~MyServo();

    PACKET* dataToSendServo;

    uint8_t gripperPos;
    uint8_t lockerPos;

    void initServo();
    void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond);
    void openServo();
    void closeServo();
    void pushServo();
    void releaseServo();
};

MyServo::MyServo(PACKET* _packet)
{
    dataToSendServo = _packet;

}

MyServo::~MyServo()
{
}

#endif