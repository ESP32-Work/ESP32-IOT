Collecting workspace information# Simple Unit Test Project

This project demonstrates how to use the Unity framework for testing C++ and embedded code on the ESP32 microcontroller. It includes examples of unit tests for both native and ESP32 environments.

## Overview

The project contains two main parts:
1. **Native Environment**: Unit tests run on a desktop environment, emulating the device.
2. **ESP32 Environment**: Unit tests run on the actual ESP32 hardware.

## Project Structure

```
.gitignore
.pio/
.vscode/
include/
lib/
src/
test/
platformio.ini
readme.md
```

## Native Environment

In the native environment, the tests are run on a desktop, and the code is written like standard C++ code without the `setup` and `loop` functions.

### Example Code

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

### PlatformIO Configuration

```ini
[env:native]
platform = native
```

## ESP32 Environment

In the ESP32 environment, the tests are run on the actual ESP32 hardware, and the code includes the `setup` and `loop` functions.

### Example Code

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

### PlatformIO Configuration

```ini
[env:ESP32_S3_DEV_4MB_QD_No_PSRAM]
platform = espressif32
board = ESP32_S3_DEV_4MB_QD_No_PSRAM
framework = arduino
test_build_src = true  ; Include the main project source in test builds
monitor_speed = 115200 ; Serial monitor speed
test_ignore = test_native_main.cpp
```


## License

This project is open-source and available under the MIT License.

## Contributing

Contributions are welcome! Feel free to open a pull request or issue.

## Contact

For any questions or issues, please open an issue on the project's GitHub page.