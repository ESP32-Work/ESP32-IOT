#include <WiFi.h>
#include "esp_wifi.h"

// Beacon Packet buffer
uint8_t packet[128] = { 
  0x80, 0x00,             // Frame Control
  0x00, 0x00,             // Duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   // Destination address
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // Source address
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // BSSID
  0x00, 0x00,             // Sequence Control
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp
  0x64, 0x00,             // Beacon Interval
  0x31, 0x04,             // Capability info
  0x00                    // SSID Parameter
};

char ssids[10][32] = {
  "Network_1",
  "Free_WiFi_2",
  "OpenNet_3",
  "Public_4",
  "WiFi_5",
  "Internet_6",
  "Access_7",
  "Hotspot_8",
  "Web_9",
  "Connect_10"
};

bool broadcasting = false;
String serialCommand;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_MODE_AP);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_start();
  
  Serial.println("Beacon Spam Tool");
  Serial.println("Commands:");
  Serial.println("start - Start broadcasting");
  Serial.println("stop - Stop broadcasting");
  Serial.println("list - List current SSIDs");
  Serial.println("set <position> <name> - Set new SSID (position 0-9)");
  Serial.println("status - Show current status");
}

void handleSerialCommands() {
  if (Serial.available()) {
    serialCommand = Serial.readStringUntil('\n');
    serialCommand.trim();
    
    if (serialCommand == "start") {
      broadcasting = true;
      Serial.println("Broadcasting started");
    }
    else if (serialCommand == "stop") {
      broadcasting = false;
      Serial.println("Broadcasting stopped");
    }
    else if (serialCommand == "list") {
      Serial.println("Current SSIDs:");
      for (int i = 0; i < 10; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(ssids[i]);
      }
    }
    else if (serialCommand == "status") {
      Serial.print("Broadcasting: ");
      Serial.println(broadcasting ? "Active" : "Stopped");
    }
    else if (serialCommand.startsWith("set ")) {
      int pos = serialCommand.substring(4, 5).toInt();
      String newSSID = serialCommand.substring(6);
      
      if (pos >= 0 && pos < 10 && newSSID.length() > 0 && newSSID.length() < 32) {
        newSSID.toCharArray(ssids[pos], 32);
        Serial.print("Updated SSID at position ");
        Serial.print(pos);
        Serial.print(" to: ");
        Serial.println(ssids[pos]);
      } else {
        Serial.println("Invalid position or SSID length");
      }
    }
  }
}

void loop() {
  handleSerialCommands();
  
  if (broadcasting) {
    for(int i = 0; i < 10; i++) {
      // Set random MAC address
      packet[10] = packet[16] = random(256);
      packet[11] = packet[17] = random(256);
      packet[12] = packet[18] = random(256);
      packet[13] = packet[19] = random(256);
      packet[14] = packet[20] = random(256);
      packet[15] = packet[21] = random(256);
      
      // Set SSID length
      int ssidLen = strlen(ssids[i]);
      packet[37] = ssidLen;
      
      // Set SSID
      for(int j = 0; j < ssidLen; j++) {
        packet[38 + j] = ssids[i][j];
      }
      
      // Send packet
      esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
      delay(1);
    }
  }
}