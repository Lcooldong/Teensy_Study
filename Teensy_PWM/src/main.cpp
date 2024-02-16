#include <Arduino.h>


void setup() {
  Serial.begin(115200);
  

}

void loop() {

  for (int i = 0; i < 256; i++)
  {
    analogWrite(LED_BUILTIN, i);
    Serial.println(i);
    delay(10);
  }
  for (int i = 255; i > 0; i--)
  {
    analogWrite(LED_BUILTIN, i);
    Serial.println(i);
    delay(10);
  }
  
}

