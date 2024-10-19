#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Insert Firebase project API Key
#define API_KEY ""

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "" 

#define LED_PIN 2 // Built-in LED pin for ESP32

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long readDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK) {
    unsigned long currentMillis = millis();

    // Send WiFi RSSI and MAC address every 1 second
    if (currentMillis - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0) {
      sendDataPrevMillis = currentMillis;
      
      int rssi = WiFi.RSSI();
      String macAddress = WiFi.macAddress();

      if (Firebase.RTDB.setInt(&fbdo, "esp32/rssi", rssi)) {
        Serial.println("RSSI sent successfully");
      } else {
        Serial.println("Failed to send RSSI");
        Serial.println("Reason: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "esp32/mac", macAddress)) {
        Serial.println("MAC address sent successfully");
      } else {
        Serial.println("Failed to send MAC address");
        Serial.println("Reason: " + fbdo.errorReason());
      }

      Serial.printf("RSSI: %d dBm, MAC: %s\n", rssi, macAddress.c_str());
    }

    // Read LED control input every 5 seconds
    if (currentMillis - readDataPrevMillis > 5000 || readDataPrevMillis == 0) {
      readDataPrevMillis = currentMillis;

      if (Firebase.RTDB.getBool(&fbdo, "esp32/led_control")) {
        bool ledState = fbdo.boolData();
        digitalWrite(LED_PIN, ledState);
        Serial.printf("LED state updated: %s\n", ledState ? "ON" : "OFF");
      } else {
        Serial.println("Failed to read LED control state");
        Serial.println("Reason: " + fbdo.errorReason());
      }
    }
  }
}
