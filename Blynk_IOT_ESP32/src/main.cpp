#include <WiFi.h>
#include "BlynkSimpleEsp32.h"

#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME " "
#define BLYNK_AUTH_TOKEN ""

BlynkTimer timer;
// WiFi credentials
const char *ssid = "";
const char *password = "";

// Blynk authentication token
char auth[] = BLYNK_AUTH_TOKEN;

// put function definitions here:
void sendDataSensor1() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("Sensor1")) {
      int value = command.substring(7).toInt();
      // Send data to Blynk server
      Serial.print("Sensor1 value: ");
      Serial.println(value);
      Blynk.virtualWrite(V1, value); // Assuming V1 is the virtual pin to send the data to
    }
  }
}

void sendDataSensor2() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("Sensor2 Value1")) {
      int value1 = command.substring(14).toInt();
      // Send data to Blynk server
      Serial.print("Sensor2 value1: ");
      Serial.println(value1);
      Blynk.virtualWrite(V2, value1); // Assuming V2 is the virtual pin to send the data to
    }
    if (command.startsWith("Sensor2 Value2")) {
      int value2 = command.substring(14).toInt();
      // Send data to Blynk server
      Serial.print("Sensor2 value2: ");
      Serial.println(value2);
      Blynk.virtualWrite(V3, value2); // Assuming V3 is the virtual pin to send the data to
    }
  }
}

void setup() {
  // WiFi connectivity check code
  Serial.begin(115200);
  Serial.println("Connecting to Blynk...");
  Blynk.begin(auth, ssid, password,IPAddress(128,199,144,129), 8080);
  while (Blynk.connected() == false) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nBlynk connected!");
  // Rest of your setup code

  timer.setInterval(2000L, sendDataSensor1);
  timer.setInterval(1000L, sendDataSensor2);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}

