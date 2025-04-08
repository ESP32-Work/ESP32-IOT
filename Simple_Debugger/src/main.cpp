#include <Arduino.h>

const int i = 0;
int j = 0;

void setup() 
{
  Serial.begin(115200);
  pinMode(RGB_BUILTIN, OUTPUT);
}


void loop() 
{
  Serial.println("i = " + i);
  j++;
  Serial.println("j = " + j);
  delay(1000);
  if (j > 10) {
    j = 0;
    digitalWrite(RGB_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(RGB_BUILTIN, LOW);
  }
} 