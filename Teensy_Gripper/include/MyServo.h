#ifndef __MYSERVO_H__
#define __MYSERVO_H__

#include <PWMServo.h>
#include "config.h"

class MyServo
{
private:
    PWMServo gripperServo;
    PWMServo lockerServo;
public:
    MyServo(/* args */);
    ~MyServo();

    uint8_t gripperPos;
    uint8_t lockerPos;

    void initServo();
    void rotateServo(PWMServo *_servo, int targetPos, uint32_t millisecond);
    void openServo();
    void openServo(bool hallRangeOn);
    void closeServo();
    void pushServo();
    void releaseServo();
};



#endif