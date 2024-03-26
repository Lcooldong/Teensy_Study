#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define OLD_PINOUT
#define NEW_PINOUT

#define HWSERIAL Serial2  // RX2 : 7 (Green), TX2 : 8 (White)


#define COLOR_Y_MIN_VALUE 250
#define COLOR_Y_MAX_VALUE 550

#define HALL_MID_VALUE    600
#define HALL_TARGET_VALUE 300

// #define HALL_FAR      0xF1
// #define HALL_NEARBY   0x04
// #define HALL_ARRIVED  0x05



#define COLOR_ON      0x07
#define COLOR_OFF     0xF3

#define SERVO_INITIAL_POS  0
#define SERVO_TARGET_POS  70

#define SERVO2_INITIAL_POS 70
#define SERVO2_TARGET_POS  35



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
const int Servo_Pin = 18;
const int Servo2_Pin = 19;

const int singNeopixel_Pin = 2;
const int ringNeopixel_Pin = 3;

//const int SDA_Pin = 20;
//const int SCL_Pin = 21;

const int hallSensor_Pin = 22;
const int button_Pin = 23;
#endif

const int blinkInterval =  300;
const int serialInterval = 1000;

typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t response;
  uint8_t servoState;
  uint8_t hallState;
  uint8_t colorState;
  uint8_t lockerState;
  uint8_t checksum;
  uint8_t etx;
}PACKET;

typedef enum Hall_State
{
  HALL_FAR     = 0xF1,
  HALL_NEARBY  = 0x04,
  HALL_ARRIVED = 0x05
}HALL_STATE;


typedef enum ToggleFlag
{
  OFF = 0x00,
  ON  = 0x01
}TOGGLE_FLAG;

typedef enum Response_State
{
  RESPONSE_INIT           = 0x00,
  RESPONSE_SERVO_OPEN     = 0x01,
  RESPONSE_SERVO_CLOSE    = 0x02,
  RESPONSE_LOCKER_RELEASE = 0x03,
  RESPONSE_LOCKER_PUSH    = 0x04,
  RESPONSE_HALL_ON        = 0x05,
  RESPONSE_HALL_OFF       = 0x06,
  RESPONSE_COLOER_ON      = 0x07,
  RESPONSE_COLOER_OFF     = 0x08,
}RESPONSE_STATE;

#endif