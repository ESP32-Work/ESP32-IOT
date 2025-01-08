- this code is a simple example of how to use unity framework for testing cpp code
- when use the native env be sure to not have setup and loop in test code just a main
- as the device is not available in the native env and we are emulating the device
- the code is being run on desktop so it will need to be like a normal cpp code

```cpp
#include <unity.h>

// Function to be tested
int add(int a, int b) {
    return a + b;
}

// Test case 1: Test addition
void test_addition() {
    TEST_ASSERT_EQUAL(5, add(2, 3));
    TEST_ASSERT_EQUAL(-1, add(-2, 1));
    TEST_ASSERT_EQUAL(0, add(0, 0));
}

int main() {
    UNITY_BEGIN();  // Initialize Unity

    RUN_TEST(test_addition);  // Run the test

    UNITY_END();  // End Unity

    return 0;
}
```
```ini

[env:native]
platform = native

```





- this code is a simple example of how to use unity framework for testing embedded code
- when use the esp32 env be sure to have setup and loop in test code
- this is because the esp32 env is a real device and the code will be run on the device
- the code is being run on the device so it will need to be like a normal arduino code
- 

```cpp
#include <Arduino.h>
#include <unity.h>
#include "WiFiManager.h"

WiFiManager wifiManager;

// Mock Wi-Fi credentials
const char* TEST_SSID = "Testwifi";
const char* TEST_PASSWORD = "x11y22z33";

// Test: Wi-Fi connection timeout
void test_wifi_connection_timeout() {
    wifiManager.begin(TEST_SSID, TEST_PASSWORD);
    TEST_ASSERT_FALSE(wifiManager.isConnected());
}

// Test: Get IP address when disconnected
void test_get_ip_address_disconnected() {
    String ip = wifiManager.getIPAddress();
    TEST_ASSERT_EQUAL_STRING("Not Connected", ip.c_str());
}

void setup() {
    UNITY_BEGIN();

    RUN_TEST(test_wifi_connection_timeout);
    RUN_TEST(test_get_ip_address_disconnected);

    UNITY_END();
}

void loop() {
    // Not used for unit testing
}
```
```ini
[env:ESP32_S3_DEV_4MB_QD_No_PSRAM]
platform = espressif32
board = ESP32_S3_DEV_4MB_QD_No_PSRAM
framework = arduino
test_build_src = true  ; Include the main project source in test builds
monitor_speed = 115200         ; Serial monitor speed
test_ignore = test_native_main.cpp
```
