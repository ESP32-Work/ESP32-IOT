#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(80);

// HTML content from previous response (stored in PROGMEM to save RAM)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Signal Radar</title>
    <style>
        body {
            margin: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            background: #1a1a1a;
            font-family: Arial, sans-serif;
        }

        .radar-container {
            position: relative;
            width: 400px;
            height: 400px;
        }

        .radar-background {
            position: absolute;
            width: 100%;
            height: 100%;
            border-radius: 50%;
            border: 2px solid #333;
            background: radial-gradient(circle, #000 0%, #111 100%);
        }

        .radar-circles {
            position: absolute;
            width: 100%;
            height: 100%;
        }

        .radar-circle {
            position: absolute;
            border: 1px solid #333;
            border-radius: 50%;
        }

        .radar-line {
            position: absolute;
            width: 50%;
            height: 2px;
            background: linear-gradient(90deg, rgba(0,255,0,0.5) 0%, rgba(0,255,0,0) 100%);
            transform-origin: 100% 50%;
            top: 50%;
            right: 50%;
            animation: radar-sweep 10s infinite linear;
        }

        .signal-points {
            position: absolute;
            width: 100%;
            height: 100%;
        }

        .signal-point {
            position: absolute;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            transform: translate(-50%, -50%);
            animation: signal-pulse 2s infinite;
        }

        @keyframes radar-sweep {
            from {
                transform: rotate(0deg);
            }
            to {
                transform: rotate(360deg);
            }
        }

        @keyframes signal-pulse {
            0% {
                opacity: 1;
                transform: translate(-50%, -50%) scale(1);
            }
            100% {
                opacity: 0;
                transform: translate(-50%, -50%) scale(2);
            }
        }

        .network-list {
            position: absolute;
            left: 420px;
            top: 0;
            color: #0f0;
            background: rgba(0, 0, 0, 0.8);
            padding: 20px;
            border-radius: 10px;
            max-height: 400px;
            overflow-y: auto;
        }
    </style>
</head>
<body>
    <div class="radar-container">
        <div class="radar-background"></div>
        <div class="radar-circles">
            <div class="radar-circle" style="width: 25%; height: 25%; left: 37.5%; top: 37.5%"></div>
            <div class="radar-circle" style="width: 50%; height: 50%; left: 25%; top: 25%"></div>
            <div class="radar-circle" style="width: 75%; height: 75%; left: 12.5%; top: 12.5%"></div>
        </div>
        <div class="radar-line"></div>
        <div class="signal-points"></div>
        <div class="network-list"></div>
    </div>

    <script>
        function updateSignalPoints() {
            fetch('/scan')
                .then(response => response.json())
                .then(networks => {
                    const signalPoints = document.querySelector('.signal-points');
                    signalPoints.innerHTML = '';
                    
                    networks.forEach(network => {
                        // Convert signal strength (-100 to 0) to distance (0 to 1)
                        const distance = (100 + network.signal) / 100;
                        // Convert to polar coordinates
                        const angle = Math.random() * Math.PI * 2;
                        const x = 200 + Math.cos(angle) * (distance * 200);
                        const y = 200 + Math.sin(angle) * (distance * 200);
                        
                        // Create signal point
                        const point = document.createElement('div');
                        point.className = 'signal-point';
                        point.style.left = `${x}px`;
                        point.style.top = `${y}px`;
                        
                        // Color based on signal strength
                        const hue = 120 * (distance);
                        point.style.backgroundColor = `hsla(${hue}, 100%, 50%, 0.8)`;
                        
                        signalPoints.appendChild(point);
                    });

                    // Update network list
                    const networkList = document.querySelector('.network-list');
                    networkList.innerHTML = '<h3>Detected Networks</h3>' + 
                        networks.map(network => `
                            <div style="margin-bottom: 10px;">
                                <div>${network.ssid}</div>
                                <div style="color: ${getSignalColor(network.signal)}">
                                    Signal: ${network.signal} dBm
                                </div>
                                <div style="color: #888">
                                    ${network.encryption} - Ch ${network.channel}
                                </div>
                            </div>
                        `).join('');
                })
                .catch(error => console.error('Error fetching networks:', error));
        }

        function getSignalColor(signal) {
            const strength = (100 + signal) / 100;
            const hue = 120 * strength;
            return `hsl(${hue}, 100%, 50%)`;
        }

        // Update every 10 seconds
        setInterval(updateSignalPoints, 10000);
        updateSignalPoints(); // Initial update
    </script>
</body>
</html>
)rawliteral";

// Helper function to determine the encryption type (your existing function)
const char* encryptionType(wifi_auth_mode_t encryption) {
  switch (encryption) {
    case WIFI_AUTH_OPEN: return "Open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Enterprise";
    default: return "Unknown";
  }
}

// Handle root URL
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Handle WiFi scan request
void handleScan() {
  // Create JSON array for network data
  StaticJsonDocument<4096> doc;
  JsonArray networks = doc.to<JsonArray>();

  int numNetworks = WiFi.scanNetworks();
  
  if (numNetworks > 0) {
    for (int i = 0; i < numNetworks; i++) {
      JsonObject network = networks.createNestedObject();
      network["ssid"] = WiFi.SSID(i);
      network["signal"] = WiFi.RSSI(i);
      network["encryption"] = encryptionType(WiFi.encryptionType(i));
      network["channel"] = WiFi.channel(i);
    }
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  
  // Clean up scan results
  WiFi.scanDelete();
}

void setup() {
  Serial.begin(115200);
  
  // Set up WiFi Access Point
  WiFi.softAP("ESP32-WIFI-RADAR", "");
  
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}