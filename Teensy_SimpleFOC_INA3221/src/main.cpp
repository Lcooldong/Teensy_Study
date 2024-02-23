#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <SDL_Arduino_INA3221.h>


uint64_t ina3221_Time = 0;
uint16_t ina3221_Interval = 100;

uint64_t ledBreathe_Time = 0;
uint16_t ledBreathe_Interval = 10;
uint8_t led_Value = 0;
bool led_Direction = false;

SDL_Arduino_INA3221 ina3221;


void getINA3221Data(uint16_t _interval);
void ledBreathe(uint16_t _interval);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();   // I2C0 SDA0(18) , SCL0(19)
 
  ina3221.begin();

  Serial.print("Manufactures ID=0x");
  int MID;
  MID = ina3221.getManufID();
  Serial.println(MID,HEX);

}

void loop() {

  ledBreathe(ledBreathe_Interval);

  // INA3221
  getINA3221Data(ina3221_Interval);

    

  // FOC 

}

void getINA3221Data(uint16_t _interval)
{
  if(millis() - ina3221_Time > _interval)
  {

    

    float shuntvoltage1 = 0;
    float busvoltage1 = 0;
    float current_mA1 = 0;
    float loadvoltage1 = 0;

    busvoltage1 = ina3221.getBusVoltage_V(1);
    shuntvoltage1 = ina3221.getShuntVoltage_mV(1);
    current_mA1 = -ina3221.getCurrent_mA(1);  // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
    loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
    
    float shuntvoltage2 = 0;
    float busvoltage2 = 0;
    float current_mA2 = 0;
    float loadvoltage2 = 0;

    busvoltage2 = ina3221.getBusVoltage_V(2);
    shuntvoltage2 = ina3221.getShuntVoltage_mV(2);
    current_mA2 = -ina3221.getCurrent_mA(2);
    loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000);
    
    float shuntvoltage3 = 0;
    float busvoltage3 = 0;
    float current_mA3 = 0;
    float loadvoltage3 = 0;

    busvoltage3 = ina3221.getBusVoltage_V(3);
    shuntvoltage3 = ina3221.getShuntVoltage_mV(3);
    current_mA3 = ina3221.getCurrent_mA(3);
    loadvoltage3 = busvoltage3 + (shuntvoltage3 / 1000);
    

    Serial.println("------------------------------");

    Serial.print("LIPO_Battery Bus Voltage:   "); Serial.print(busvoltage1); Serial.println(" V");
    Serial.print("LIPO_Battery Shunt Voltage: "); Serial.print(shuntvoltage1); Serial.println(" mV");
    Serial.print("LIPO_Battery Load Voltage:  "); Serial.print(loadvoltage1); Serial.println(" V");
    Serial.print("LIPO_Battery Current 1:       "); Serial.print(current_mA1); Serial.println(" mA");
    Serial.println("");

    Serial.print("Solar Cell Bus Voltage 2:   "); Serial.print(busvoltage2); Serial.println(" V");
    Serial.print("Solar Cell Shunt Voltage 2: "); Serial.print(shuntvoltage2); Serial.println(" mV");
    Serial.print("Solar Cell Load Voltage 2:  "); Serial.print(loadvoltage2); Serial.println(" V");
    Serial.print("Solar Cell Current 2:       "); Serial.print(current_mA2); Serial.println(" mA");
    Serial.println("");

    Serial.print("Output Bus Voltage 3:   "); Serial.print(busvoltage3); Serial.println(" V");
    Serial.print("Output Shunt Voltage 3: "); Serial.print(shuntvoltage3); Serial.println(" mV");
    Serial.print("Output Load Voltage 3:  "); Serial.print(loadvoltage3); Serial.println(" V");
    Serial.print("Output Current 3:       "); Serial.print(current_mA3); Serial.println(" mA");
    Serial.println("");

    ina3221_Time = millis();
  }
}

void ledBreathe(uint16_t _interval)
{

  if(millis() - ledBreathe_Time > _interval)
  {
    
    if(led_Direction == false)
    {
      led_Value ++;
    }
    else
    {
      led_Value --;
    }

    if(led_Value == 255 || led_Value == 0)
    {
      led_Direction = !led_Direction;
    }

    // Serial.printf("Value : %d\r\n", led_Value);
    analogWrite(LED_BUILTIN, led_Value);
    
    ledBreathe_Time = millis();
  }

}