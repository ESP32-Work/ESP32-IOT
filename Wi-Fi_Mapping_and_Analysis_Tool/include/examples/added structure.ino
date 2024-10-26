#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(80);

struct NetworkInfo {
    String ssid;
    String bssid;
    int32_t signal;
    String encryption;
    uint8_t channel;
    unsigned long lastSeen;
    double distance;
};

std::vector<NetworkInfo> networks;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            background-color: #000; 
            color: #00ff00;
            font-family: Arial, sans-serif;
            margin: 20px;
        }
        .container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        .panel {
            background-color: #111;
            border: 1px solid #00ff00;
            padding: 15px;
            border-radius: 8px;
        }
        .radar {
            position: relative;
            width: 400px;
            height: 400px;
            border-radius: 50%;
            border: 2px solid #00ff00;
            margin: auto;
        }
        .network-dot {
            position: absolute;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            transform: translate(-50%, -50%);
            cursor: pointer;
        }
        .network-list {
            max-height: 600px;
            overflow-y: auto;
        }
        .network-item {
            margin: 10px 0;
            padding: 10px;
            border: 1px solid #00ff00;
            border-radius: 4px;
        }
        .signal-excellent { background-color: #00ff00; }
        .signal-good { background-color: #90EE90; }
        .signal-fair { background-color: #FFD700; }
        .signal-poor { background-color: #FF4500; }
        #scanButton {
            background-color: #00ff00;
            color: black;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            cursor: pointer;
            margin-bottom: 20px;
        }
        .radar-sweep {
            position: absolute;
            width: 50%;
            height: 2px;
            background: linear-gradient(90deg, rgba(0,255,0,0.5) 0%, transparent 100%);
            transform-origin: left center;
            left: 50%;
            top: 50%;
            animation: sweep 4s infinite linear;
        }
        @keyframes sweep {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <h1>WiFi Radar System</h1>
    <button id="scanButton" onclick="startScan()">Start Scan</button>
    
    <div class="container">
        <div class="panel">
            <h2>Radar View</h2>
            <div class="radar">
                <div class="radar-sweep"></div>
                <div id="networkDots"></div>
            </div>
        </div>
        
        <div class="panel">
            <h2>Network List</h2>
            <div id="networkList" class="network-list"></div>
        </div>
    </div>

    <script>
        function getSignalClass(signal) {
            if (signal >= -50) return 'signal-excellent';
            if (signal >= -60) return 'signal-good';
            if (signal >= -70) return 'signal-fair';
            return 'signal-poor';
        }

        function updateRadar(networks) {
            const radar = document.getElementById('networkDots');
            radar.innerHTML = '';
            
            networks.forEach(network => {
                const dot = document.createElement('div');
                dot.className = `network-dot ${getSignalClass(network.signal)}`;
                
                // Convert signal strength to position
                const distance = Math.min(Math.abs(network.signal + 30) / 90, 1) * 180;
                const angle = (network.channel / 14) * 2 * Math.PI;
                
                const x = 200 + Math.cos(angle) * distance;
                const y = 200 + Math.sin(angle) * distance;
                
                dot.style.left = x + 'px';
                dot.style.top = y + 'px';
                dot.title = `${network.ssid}\nSignal: ${network.signal} dBm\nChannel: ${network.channel}`;
                
                radar.appendChild(dot);
            });
        }

        function updateNetworkList(networks) {
            const list = document.getElementById('networkList');
            list.innerHTML = networks.map(network => `
                <div class="network-item">
                    <strong>SSID:</strong> ${network.ssid || '(Hidden Network)'}<br>
                    <strong>Signal:</strong> ${network.signal} dBm<br>
                    <strong>Channel:</strong> ${network.channel}<br>
                    <strong>Encryption:</strong> ${network.encryption}<br>
                    <strong>BSSID:</strong> ${network.bssid}
                </div>
            `).join('');
        }

        function startScan() {
            document.getElementById('scanButton').disabled = true;
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    updateRadar(data);
                    updateNetworkList(data);
                    document.getElementById('scanButton').disabled = false;
                })
                .catch(error => {
                    console.error('Error:', error);
                    document.getElementById('scanButton').disabled = false;
                });
        }

        // Initial scan
        startScan();
        // Auto refresh every 10 seconds
        setInterval(startScan, 10000);
    </script>
</body>
</html>
)rawliteral";

String getEncryptionType(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN:           return "Open";
        case WIFI_AUTH_WEP:            return "WEP";
        case WIFI_AUTH_WPA_PSK:        return "WPA-PSK";
        case WIFI_AUTH_WPA2_PSK:       return "WPA2-PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:   return "WPA/WPA2-PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
        default:                       return "Unknown";
    }
}

double calculateDistance(int32_t rssi) {
    // Simple distance calculation based on RSSI
    // This is a rough approximation and would need calibration for accuracy
    const double RSSI_AT_1M = -40.0; // RSSI at 1 meter (needs calibration)
    const double N = 2.7; // Path loss exponent (adjust based on environment)
    
    return pow(10.0, (RSSI_AT_1M - rssi) / (10.0 * N));
}

void handleRoot() {
    server.send(200, "text/html", index_html);
}

void handleScan() {
    int n = WiFi.scanNetworks();
    networks.clear();
    
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            NetworkInfo network;
            network.ssid = WiFi.SSID(i);
            network.bssid = WiFi.BSSIDstr(i);
            network.signal = WiFi.RSSI(i);
            network.encryption = getEncryptionType(WiFi.encryptionType(i));
            network.channel = WiFi.channel(i);
            network.lastSeen = millis();
            network.distance = calculateDistance(network.signal);
            
            networks.push_back(network);
        }
    }
    
    String jsonString = "[";
    for (size_t i = 0; i < networks.size(); i++) {
        if (i > 0) jsonString += ",";
        jsonString += "{";
        jsonString += "\"ssid\":\"" + networks[i].ssid + "\",";
        jsonString += "\"bssid\":\"" + networks[i].bssid + "\",";
        jsonString += "\"signal\":" + String(networks[i].signal) + ",";
        jsonString += "\"encryption\":\"" + networks[i].encryption + "\",";
        jsonString += "\"channel\":" + String(networks[i].channel) + ",";
        jsonString += "\"distance\":" + String(networks[i].distance);
        jsonString += "}";
    }
    jsonString += "]";
    
    server.send(200, "application/json", jsonString);
}

void setup() {
    Serial.begin(115200);
    
    // Set up AP mode
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("WiFi-Radar", "");
    
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    server.on("/", handleRoot);
    server.on("/scan", handleScan);
    server.begin();
    
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}