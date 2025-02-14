#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

const char* ssid = "Testwifi";
const char* password = "x11y22z33";
const char* baseHostname = "esp32";

WebServer server(80);
String hostname;

String generateHostname() {
    // Use the last 3 bytes of MAC address to create a unique hostname
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char chipId[7];
    snprintf(chipId, 7, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(baseHostname) + "-" + String(chipId);
}

void setup() {
    Serial.begin(115200);
    
    // Generate unique hostname
    hostname = generateHostname();
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    
    // Initialize mDNS with unique hostname
    if (!MDNS.begin(hostname.c_str())) {
        Serial.println("Error setting up mDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");
    Serial.print("Device hostname: ");
    Serial.println(hostname + ".local");
    
    // Add service to mDNS
    MDNS.addService("_http", "_tcp", 80);
    
    // Set up web server routes
    server.on("/", []() {
        String html = "<h1>Hello from " + hostname + "</h1>";
        html += "<p>Access this device using: " + hostname + ".local</p>";
        server.send(200, "text/html", html);
    });
    
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}