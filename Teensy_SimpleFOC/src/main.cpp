#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>

#define DRIVER_VOLTAGE 12

uint64_t ledLastTime = 0;
bool ledDirection = false;
uint8_t ledPWM = 0;
uint8_t ledInterval = 20; 

void breathe();


const int sensor1_PIN = 21;
float target_position = 0;
float target_velocity = 0;

BLDCMotor motor1 = BLDCMotor(7);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(2, 3, 4);

MagneticSensorPWM sensor1 = MagneticSensorPWM(sensor1_PIN, 4, 904);

void doPWM1(){sensor1.handlePWM();}
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_position, cmd); }
void doLimit(char* cmd) { command.scalar(&motor1.voltage_limit, cmd); }
void doVelocity(char* cmd) { command.scalar(&motor1.velocity_limit, cmd); }


void setup() {
  Serial.begin(115200);
  
  sensor1.init();
  // motor1.linkSensor(&sensor1);

  driver1.voltage_power_supply = DRIVER_VOLTAGE;
  driver1.voltage_limit = 6;
  driver1.init();
  motor1.linkDriver(&driver1);

  // motor1.controller = MotionControlType::velocity;

  // motor1.PID_velocity.P = 0.2f;
  // motor1.PID_velocity.I = 20;
  // motor1.PID_velocity.D = 0;
  // default voltage_power_supply
  motor1.voltage_limit = 2;
  motor1.velocity_limit = 5;

  // motor1.PID_velocity.output_ramp = 1000;
  // motor1.LPF_velocity.Tf = 0.01f;

  motor1.useMonitoring(Serial);

  
  // sensor1.enableInterrupt(doPWM1);
  motor1.controller = MotionControlType::angle_openloop;

  motor1.init();
  // motor1.initFOC();

  command.add('T', doTarget, "target angle");
  command.add('L', doLimit, "voltage limit");
  command.add('V', doLimit, "movement velocity");

  _delay(1000);
}

void loop() {

  breathe();

  // motor1.loopFOC();
  motor1.move(target_position);
  command.run();

  // sensor1.update();
  // Serial.print(sensor1.getAngle());
  // Serial.println();

}






void breathe()
{
  if(millis() - ledLastTime > ledInterval)
  {
    if(ledDirection == false)
    {
      ledPWM++;
    }
    else
    {
      ledPWM--;
    }

    if(ledPWM <= 0)
    {
      ledDirection = false;
    }
    else if(ledPWM >= 255)
    {
      ledDirection = true;
    }

    analogWrite(LED_BUILTIN, ledPWM);
    // Serial.printf("%d\r\n", ledPWM);

    ledLastTime = millis();
  }
}


