#include "WiFiManager.h"

WiFiManager::WiFiManager() {}

void WiFiManager::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    WiFi.begin(ssid, password);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > 10000) {
            break; // Timeout after 10 seconds
        }
        delay(500);
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    } else {
        return "Not Connected";
    }
}
