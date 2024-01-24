#ifndef __MYSERVO_H__
#define __MYSERVO_H__


#include <PWMServo.h>



class MyServo
{
private:
    PWMServo gripperServo;
    PWMServo lockerServo;

public:
    MyServo();
    ~MyServo();

    uint8_t gripperPos;
    uint8_t lockerPos;

    void initServo();
    void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond);
    void openServo();
    void closeServo();
    void pushServo();
    void releaseServo();
};

MyServo::MyServo()
{
   
}

MyServo::~MyServo()
{
}

#endif