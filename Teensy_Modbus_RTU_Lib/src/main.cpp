#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>



// Not working Yet

#define SLAVE_ID 1

SoftwareSerial RS485(0, 1); // RX,TX

//TODO
ModbusRTUSlave modbus(Serial1, 0);


uint64_t lastTime = 0;
uint64_t count = 0;

bool coils[2];
bool discreteInputs[2];
uint16_t holdingRegisters[2];
uint16_t inputRegisters[2];


void setup() {
  Serial.begin(115200);
  while(!Serial){}

  modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
  modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs
  modbus.configureHoldingRegisters(holdingRegisters, 2); // unsigned 16 bit integer array of holding register values, number of holding registers
  modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers

  modbus.begin(1, 9600);

  Serial.println("Start Teensy Modbus");
  

}

void loop() {
  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;
    holdingRegisters[0] = count;
    holdingRegisters[1] = count + 1;
    Serial.printf("Holding[%d] %d\r\n", 0, count);
    
  }

  
  modbus.poll();


}

