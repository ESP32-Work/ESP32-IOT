# ESP32 Firebase IoT Project

## Overview

This project demonstrates how to use an ESP32 microcontroller with Firebase Realtime Database to create a simple IoT system. The ESP32 reports its WiFi signal strength (RSSI) and MAC address to Firebase every second, and controls its built-in LED based on a value read from Firebase every 5 seconds.

## Features

- WiFi connection management
- Firebase integration
- Real-time reporting of WiFi RSSI
- Real-time reporting of ESP32's MAC address
- Remote control of ESP32's built-in LED via Firebase
- Non-blocking code design using millis() for timing

## Hardware Requirements

- ESP32 development board (e.g., ESP32 DevKit, NodeMCU-32S)
- Micro-USB cable for programming and power
- (Optional) External LED and appropriate resistor if not using built-in LED

## Software Requirements

- Arduino IDE (1.8.x or later)
- ESP32 board support for Arduino IDE
- Firebase ESP32 Client library by Mobizt

## Setup Instructions

1. **Library Installation**
   - Search for "Firebase ESP32 Client" and install the library by Mobizt

2. **Firebase Setup**
   - Go to [Firebase Console](https://console.firebase.google.com/)
   - Create a new project (or use an existing one)
   - In the project settings, find your Web API Key
   - In the Realtime Database section, create a new database and note the database URL
   - Set up database rules to allow read and write access

3. **Code Configuration**
   - Open the provided Arduino sketch
   - Replace the placeholders in the code with your actual WiFi and Firebase credentials:
     ```cpp
     #define WIFI_SSID "Your_WiFi_SSID"
     #define WIFI_PASSWORD "Your_WiFi_Password"
     #define API_KEY "Your_Firebase_API_Key"
     #define DATABASE_URL "Your_Firebase_Database_URL"
     ```

4. **Upload the Code**
   - Connect your ESP32 to your computer
   - Select the correct board and port in Arduino IDE
   - Click the Upload button

## Usage

1. After uploading, open the Serial Monitor to view debug information
2. In your Firebase Realtime Database, you'll see new data under:
   - `esp32/rssi`: The current WiFi signal strength (updates every second)
   - `esp32/mac`: The MAC address of your ESP32 (updates every second)
3. To control the LED:
   - In your Firebase database, create a boolean value at `esp32/led_control`
   - Set it to `true` to turn the LED on, or `false` to turn it off
   - The ESP32 will check this value every 5 seconds and update the LED accordingly

## Troubleshooting

- If you're not seeing data in Firebase, check your WiFi credentials and Firebase configuration
- Ensure your Firebase database rules allow read and write access
- Check the Serial Monitor for any error messages or connection issues

## Extending the Project

- Add more sensors to report additional data to Firebase
- Implement more controls from Firebase to ESP32
- Create a web or mobile app to visualize the data and control the ESP32

## License

This project is open-source and available under the MIT License.

## Contributors

- Ibrahim Bin Mansur - Initial work

## Acknowledgments

- Firebase ESP32 Client library by Mobizt
- ESP32 community for their extensive documentation and examples