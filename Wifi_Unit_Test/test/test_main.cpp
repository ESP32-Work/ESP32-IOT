#include <unity.h>
#include <Arduino.h>
#include "wifi.h"

typedef void (*WiFiEventCb_t)(WiFiEvent_t);

// Mock WiFi class
class MockWiFiClass {
public:
    static bool _connected;
    static IPAddress _localIP;
    static void begin(const char* ssid, const char* password) {}
    static wl_status_t status() { return _connected ? WL_CONNECTED : WL_DISCONNECTED; }
    static IPAddress localIP() { return _localIP; }
    static void onEvent(WiFiEventCb_t cb) { _eventCallback = cb; }
    static void triggerEvent(WiFiEvent_t event) {
        if (_eventCallback) _eventCallback(event);
    }
private:
    static WiFiEventCb_t _eventCallback;
};

// Initialize static members
bool MockWiFiClass::_connected = false;
IPAddress MockWiFiClass::_localIP(192, 168, 1, 100);
WiFiEventCb_t MockWiFiClass::_eventCallback = nullptr;

// Mock Serial class
class SerialClass {
public:
    static String _printOutput;
    static void print(const char* message) { _printOutput += message; }
    static void print(IPAddress ip) { _printOutput += ip.toString(); }
    static void println(const char* message) { _printOutput += message; _printOutput += "\n"; }
    static void println(IPAddress ip) { _printOutput += ip.toString(); _printOutput += "\n"; }
    static void clear() { _printOutput = ""; }
};

String SerialClass::_printOutput = "";

void setUp(void) {
    // Reset state before each test
    MockWiFiClass::_connected = false;
    SerialClass::clear();
}

void tearDown(void) {
    // Clean up after each test
}

void test_connect_to_wifi_successful(void) {
    // Simulate successful connection after 2 attempts
    MockWiFiClass::_connected = false;
    int attempts = 0;
    
    // Create a timer to simulate WiFi connecting after 2 seconds
    TEST_ASSERT_EQUAL(WL_DISCONNECTED, WiFi.status());
    delay(2000);
    MockWiFiClass::_connected = true;
    
    connectToWiFi();
    TEST_ASSERT_EQUAL(WL_CONNECTED, WiFi.status());
    TEST_ASSERT_TRUE(SerialClass::_printOutput.indexOf("Connected to WiFi") >= 0);
}

void test_reconnect_wifi_when_disconnected(void) {
    // Test reconnection when WiFi is disconnected
    MockWiFiClass::_connected = false;
    reconnectWiFi();
    TEST_ASSERT_TRUE(SerialClass::_printOutput.indexOf("WiFi connection lost") >= 0);
}

void test_wifi_event_handler_got_ip(void) {
    // Test handling of SYSTEM_EVENT_STA_GOT_IP event
    handleWiFiEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP);
    TEST_ASSERT_TRUE(SerialClass::_printOutput.indexOf("WiFi connected. IP address:") >= 0);
    TEST_ASSERT_TRUE(SerialClass::_printOutput.indexOf("192.168.1.100") >= 0);
}

void test_wifi_event_handler_disconnected(void) {
    // Test handling of SYSTEM_EVENT_STA_DISCONNECTED event
    handleWiFiEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    TEST_ASSERT_TRUE(SerialClass::_printOutput.indexOf("WiFi lost connection") >= 0);
}

void test_setup_wifi(void) {
    MockWiFiClass::_connected = true;
    setupWiFi();
    TEST_ASSERT_EQUAL(WL_CONNECTED, WiFi.status());
}

void setup() {
    delay(2000);  // Give the board time to settle
    UNITY_BEGIN();
    
    RUN_TEST(test_connect_to_wifi_successful);
    RUN_TEST(test_reconnect_wifi_when_disconnected);
    RUN_TEST(test_wifi_event_handler_got_ip);
    RUN_TEST(test_wifi_event_handler_disconnected);
    RUN_TEST(test_setup_wifi);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}