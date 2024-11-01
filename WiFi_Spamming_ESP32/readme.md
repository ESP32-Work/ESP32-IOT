# ESP32 WiFi Beacon Spammer
### An Educational Project for Understanding WiFi Beacon Frames


## 📝 Description
This project demonstrates the creation and transmission of WiFi beacon frames using an ESP32 microcontroller. It simulates multiple WiFi networks by broadcasting customizable beacon frames, providing insights into WiFi protocols and network behavior.

> ⚠️ **Educational Purpose Only**: This project is intended for educational purposes to understand WiFi protocols. Using this code for malicious purposes or disrupting networks may be illegal in your jurisdiction.

## 🚀 Features
- Simulates up to 10 concurrent WiFi networks
- Real-time serial command interface
- Customizable network names (SSIDs)
- Random MAC address generation
- Interactive control system
- Low resource consumption

## 📋 Prerequisites
### Hardware
- ESP32 Development Board
- USB Cable
- Computer for programming

### Software
- Arduino IDE (1.8.x or newer)
- ESP32 Board Support Package
- Serial Terminal (built-in Arduino IDE Serial Monitor or equivalent)

## 🛠️ Installation

1. **Project Setup**   
   ```bash
   # Clone this repository
   git clone https://github.com/ESP32-Work/ESP32-IOT.git
    ```

## 📤 Upload Instructions
1. Connect ESP32 to your computer
2. Select correct board and port 
3. Click upload button
4. Wait for compilation and upload to complete
5. Open Serial Monitor (115200 baud)

## 💻 Usage
### Serial Commands
```bash
start   - Begin broadcasting networks
stop    - Stop broadcasting
list    - Show all configured SSIDs
status  - Display current broadcasting status
set X Y - Set network X (0-9) to name Y
```

### Example Usage
```bash
# Start broadcasting
start

# List all networks
list

# Change network name
set 0 NewNetwork

# Check status
status

# Stop broadcasting
stop
```

## 🔧 Customization
### Modifying Default Networks
```cpp
char ssids[10][32] = {
    "Network_1",
    "Network_2",
    // Add more networks here
};
```

### Adjusting Timing
```cpp
// In loop() function
delay(1); // Modify delay between broadcasts
```

## 📊 Technical Details
### Beacon Frame Structure
```bash
[Frame Control] [Duration] [Destination Addr] [Source Addr] [BSSID] [Sequence] [Timestamp] [Beacon Int] [Capability] [SSID]
```

### Memory Usage
- Packet Buffer: 128 bytes
- SSID Storage: 320 bytes (10 x 32)
- Variables: ~100 bytes
- Total: ~548 bytes

## 🐛 Troubleshooting

### Common Issues
1. **ESP32 Not Detected**
   - Check USB connection
   - Verify correct board selection
   - Install/reinstall ESP32 board package

2. **Compilation Errors**
   - Verify ESP32 board package installation
   - Check Arduino IDE version compatibility
   - Ensure all required libraries are installed

3. **No Networks Appearing**
   - Confirm 'start' command was sent
   - Check serial monitor baud rate (115200)
   - Verify ESP32 initialization messages

## 🤝 Contributing
Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔍 Code Documentation
### Main Functions
```cpp
void setup()          // Initialize ESP32 and WiFi
void loop()           // Main program loop
void handleCommands() // Process serial commands
```

### Key Variables
```cpp
uint8_t packet[128]   // Beacon frame buffer
char ssids[10][32]    // Network names storage
bool broadcasting     // Broadcasting state
```

## 📚 Resources
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [802.11 Beacon Frame Format](https://en.wikipedia.org/wiki/Beacon_frame)
- [Arduino IDE](https://www.arduino.cc/en/software)

## ✨ Acknowledgments
- ESP32 Development Team
- Arduino Community
- Contributors and Testers


---
Made with ❤️ for the IoT community
