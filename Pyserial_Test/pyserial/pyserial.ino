const unsigned long SEND_INTERVAL_MS = 1000;

unsigned long previousSendTime = 0;
char inputBuffer[128];
size_t inputLength = 0;

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Process incoming serial data without waiting
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') {
            inputBuffer[inputLength] = '\0';
            Serial.print("ESP32 received: ");
            Serial.println(inputBuffer);
            inputLength = 0;
        }
        else if (c != '\r') {
            if (inputLength < sizeof(inputBuffer) - 1) {
                inputBuffer[inputLength++] = c;
            }
            else {
                // Discard an oversized message
                inputLength = 0;
                Serial.println("ERROR: Command too long");
            }
        }
    }

    // Send a periodic message
    unsigned long currentTime = millis();

    if (currentTime - previousSendTime >= SEND_INTERVAL_MS) {
        previousSendTime = currentTime;
        Serial.println("Hello from the microcontroller!");
    }
}