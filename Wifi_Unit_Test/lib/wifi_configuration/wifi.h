#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>

// Default WiFi credentials
#define WIFI_SSID "Testwifi"
#define WIFI_PASSWORD "x11y22z33"

// Function declarations
void connectToWiFi();
void reconnectWiFi();
void handleWiFiEvent(WiFiEvent_t event);
void setupWiFi();

#endif // WIFI_H