#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();
    void begin(const char* ssid, const char* password);
    bool isConnected();
    String getIPAddress();

private:
    String _ssid;
    String _password;
};

#endif // WIFI_MANAGER_H
