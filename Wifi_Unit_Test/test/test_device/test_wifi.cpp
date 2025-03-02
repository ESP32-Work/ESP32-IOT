#include <Arduino.h>
#include <unity.h>
#include "wifi_config.h"

// **Mock WiFi Variables**
static wl_status_t mockWiFiStatus = WL_DISCONNECTED;
static IPAddress mockWiFiIP(192, 168, 1, 100);

// **Mock WiFi Functions**
void mockWiFiBegin(const char* ssid, const char* password) {
    Serial.print("Mock: Connecting to WiFi...");
    delay(1000);  // Simulate connection delay
    mockWiFiStatus = WL_CONNECTED;
}

wl_status_t mockWiFiStatusFunc() {  
    return mockWiFiStatus;
}

IPAddress mockLocalIP() {
    return mockWiFiIP;
}

void mockWiFiDisconnect() {
    Serial.println("Mock: Disconnecting WiFi...");
    mockWiFiStatus = WL_DISCONNECTED;
}

// **Test Cases**
void test_connectToWiFi_success() {
    mockWiFiStatus = WL_DISCONNECTED;
    mockWiFiBegin("TestSSID", "TestPassword");

    TEST_ASSERT_EQUAL(WL_CONNECTED, mockWiFiStatus);
}

void test_reconnectWiFi_whenDisconnected() {
    mockWiFiStatus = WL_DISCONNECTED;
    reconnectWiFi();  // Should call connectToWiFi()

    TEST_ASSERT_EQUAL(WL_CONNECTED, mockWiFiStatus);
}

void test_handleWiFiEvent_connected() {
    Serial.println("Testing WiFi event: ARDUINO_EVENT_WIFI_STA_GOT_IP");
    handleWiFiEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP);

    TEST_ASSERT_EQUAL(WL_CONNECTED, mockWiFiStatus);
}

void test_handleWiFiEvent_disconnected() {
    Serial.println("Testing WiFi event: ARDUINO_EVENT_WIFI_STA_DISCONNECTED");
    handleWiFiEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    TEST_ASSERT_EQUAL(WL_CONNECTED, mockWiFiStatus);  // Should reconnect
}

// **Setup for Unity Framework**
void setup() {
    Serial.begin(115200);
    UNITY_BEGIN();

    RUN_TEST(test_connectToWiFi_success);
    RUN_TEST(test_reconnectWiFi_whenDisconnected);
    RUN_TEST(test_handleWiFiEvent_connected);
    RUN_TEST(test_handleWiFiEvent_disconnected);

    UNITY_END();
}

void loop() {
    // Empty: Unity runs tests in setup() only
}
