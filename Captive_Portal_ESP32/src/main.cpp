#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// Constants and global variables
const byte DNS_PORT = 53;
const int WEBSERVER_PORT = 80;

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

DNSServer dnsServer;
WebServer server(WEBSERVER_PORT);

char ap_ssid[32];
String ap_password = "password123";

TaskHandle_t webServerTaskHandle = NULL;

void setupAP() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(ap_ssid, sizeof(ap_ssid), "esp_%02X%02X%02X%02X%02X%02X", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ap_ssid, ap_password.c_str());
  
  // DNS Server setup
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
}

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

boolean captivePortal() {
  if (!isIp(server.hostHeader())) {
    server.sendHeader("Location", String("http://") + 
                     toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");
    server.client().stop();
    return true;
  }
  return false;
}


void handleNotFound() {
  if (captivePortal()) {
    return;
  }
  
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  server.send(404, "text/plain", message);
}

void handleHomePage() {
  if (captivePortal()) {
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>ESP32 Captive Portal</title></head><body>";
  html += "<h1>Welcome to the ESP32 Captive Portal</h1>";
  html += "<p>Please connect to the WiFi network.</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setupWebServer() {
  server.on("/", handleHomePage);
  server.onNotFound(handleNotFound);
  server.begin();
}

void webServerTask(void* pvParameters) {
  setupWebServer();
  while (true) {
    dnsServer.processNextRequest();
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  
  setupAP();
  
  xTaskCreatePinnedToCore(
    webServerTask,
    "WebServerTask",
    8192,
    NULL,
    1,
    &webServerTaskHandle,
    1
  );
}

void loop() {
  
}