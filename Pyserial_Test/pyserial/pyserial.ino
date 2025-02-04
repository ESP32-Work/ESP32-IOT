// Basic communication example
void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud
  delay(1000);  // Give time for serial to initialize
}

void loop() {
  // Send data to computer
  Serial.println("Hello from ESP32!");
  delay(1000);  // Send every second

  // Check for incoming data
  if (Serial.available()) {
    String received = Serial.readStringUntil('\n');
    Serial.print("ESP32 received: ");
    Serial.println(received);
  }
}