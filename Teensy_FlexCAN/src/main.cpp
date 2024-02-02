#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <isotp.h>
#include <SPI.h>
//#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <HardwareSerial.h>
#include <EEPROM.h>

#define LED_BUILTIN 13

isotp<RX_BANKS_16, 512> tp; /* 16 slots for multi-ID support, at 512bytes buffer each payload rebuild */

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can1; // RX  TZ

void myCallback(const ISOTP_data &config, const uint8_t *buf) {
  Serial.print("ID: ");
  Serial.print(config.id, HEX);
  Serial.print("\tLEN: ");
  Serial.print(config.len);
  Serial.print("\tFINAL ARRAY: ");
  for ( int i = 0; i < config.len; i++ ) {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  } Serial.println();
}

void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print(" LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" BUS: "); Serial.print(msg.bus);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void setup() {
  Serial.begin(115200); delay(400);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Can1.begin();
  Can1.setClock(CLK_60MHz);
  
  //Can1.setBaudRate(95238);
  Can1.setBaudRate(50000);
  Can1.setMaxMB(16);
  Can1.enableFIFO();
  Can1.enableFIFOInterrupt();
  
  //Can1.onReceive(canSniff);
  tp.begin();
  tp.setWriteBus(&Can1); /* we write to this bus */
  tp.onReceive(myCallback); /* set callback */
  delay(1000);
  Serial.println("Can  Setup");
}

int count1 = 0;
int count2 = 0;
void loop() {
  static uint32_t sendTimer1 = millis();
  static uint32_t sendTimer2 = millis();
  if ( millis() - sendTimer1 > 1000 ) {
    uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
    const char b[] = "01413AAAAABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    uint8_t test[] = { 0, 1, 2, 3, 4, 5, 6}; // data in first frame
    ISOTP_data config;
    config.id = 0x123;
    config.flags.extended = 0; /* standard frame */
    config.separation_time = 10; /* time between back-to-back frames in millisec */
    // tp.write(config, buf, sizeof(buf));
    // tp.write(config, test, sizeof(test));
    tp.write(config, b, sizeof(b));
    sendTimer1 = millis();
    Serial.printf("[1] %d\r\n", count1++);
  }


  if ( millis() - sendTimer2 > 1500 ) {
    uint8_t test2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    ISOTP_data config;
    config.id = 0x456;
    config.flags.extended = 0; /* standard frame */
    config.separation_time = 10; /* time between back-to-back frames in millisec */ 
    tp.write(config, test2, sizeof(test2));
    sendTimer2 = millis();
    Serial.printf("[2] %d\r\n", count2++);
  }
}