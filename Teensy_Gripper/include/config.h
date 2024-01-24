#ifndef __CONFIG_H__
#define __CONFIG_H__



//#define OLD_PINOUT
#define NEW_PINOUT

#define HWSERIAL Serial2  // RX2 : 7, TX2 : 8



#define COLOR_Y_MIN_VALUE 250
#define COLOR_Y_MAX_VALUE 550

#define HALL_MID_VALUE 600
#define HALL_TARGET_VALUE 300

#define HALL_FAR      0xF1
#define HALL_NEARBY   0x04
#define HALL_ARRIVED  0x05



#define COLOR_ON      0x07
#define COLOR_OFF     0xF3

#define SERVO_INITIAL_POS  0
#define SERVO_TARGET_POS  70

#define SERVO2_INITIAL_POS 10
#define SERVO2_TARGET_POS 40

#define SERVO_CLOSED  0x06
#define SERVO_OPENED  0xF2

#define SERVO_RELEASE 0xF4
#define SERVO_PUSH    0x08

#define PROG_FLASH_SIZE 1024 * 900

#ifdef OLD_PINOUT
const int Servo_Pin = 14;
const int Servo2_Pin = 15;

const int singNeopixel_Pin = 16;
const int ringNeopixel_Pin = 17;

const int SDA_Pin = 18;
const int SCL_Pin = 19;

const int hallSensor_Pin = 20;
const int button_Pin = 21;
#endif

#ifdef NEW_PINOUT
const int Servo_Pin = 16;
const int Servo2_Pin = 17;

const int singNeopixel_Pin = 18;
const int ringNeopixel_Pin = 19;

const int SDA_Pin = 20;
const int SCL_Pin = 21;

const int hallSensor_Pin = 22;
const int button_Pin = 23;
#endif

const int blinkInterval =  300;
const int serialInterval = 1000;

typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t servoState;
  uint8_t hallState;
  uint8_t colorState;
  uint8_t lockerState;
  uint8_t etx;
}PACKET;




#endif