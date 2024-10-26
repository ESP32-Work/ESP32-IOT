#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>


WebServer server(80);

// Store network information
struct NetworkInfo {
  String ssid;
  int signal;
  float avgSignal;
  String encryption;
  int channel;
  int scanCount;
  unsigned long firstSeen;
  unsigned long lastSeen;
  std::vector<int> signalHistory;
  bool isHidden;
  String bssid;
  int frequency;
};

std::vector<NetworkInfo> networksList;
const int MAX_HISTORY_SIZE = 10;
const int SIGNAL_HISTORY_INTERVAL = 60000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>WiFi Radar</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px;
            background: #f0f0f0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: #2196F3;
            color: white;
            padding: 20px;
            border-radius: 5px;
            margin-bottom: 20px;
        }
        .scan-button {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
        }
        .scan-button:disabled {
            background: #cccccc;
        }
        .content-wrapper {
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
        }
        .radar-container {
            flex: 1;
            min-width: 300px;
            background: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .network-list {
            flex: 1;
            min-width: 300px;
        }
        .network-grid {
            display: grid;
            gap: 10px;
        }
        .network-card {
            background: white;
            padding: 15px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            cursor: pointer;
            transition: background-color 0.3s;
        }
        .network-card:hover {
            background-color: #f5f5f5;
        }
        .network-card.selected {
            border: 2px solid #2196F3;
        }
        .loading {
            display: none;
            margin: 20px 0;
        }
        .loading.active {
            display: block;
        }
        .signal-strength {
            display: inline-block;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            margin-right: 10px;
        }
        .excellent { background: #4CAF50; }
        .good { background: #8BC34A; }
        .fair { background: #FFC107; }
        .poor { background: #F44336; }
        #radarCanvas {
            width: 100%;
            max-width: 500px;
            height: 500px;
            margin: 0 auto;
            display: block;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>WiFi Radar</h1>
            <button onclick="startScan()" id="scanButton" class="scan-button">Start Scan</button>
        </div>
        
        <div id="loading" class="loading">
            Scanning networks... Please wait...
        </div>

        <div class="content-wrapper">
            <div class="radar-container">
                <canvas id="radarCanvas"></canvas>
            </div>
            
            <div class="network-list">
                <div id="networkList" class="network-grid"></div>
            </div>
        </div>
    </div>

    <script>
        let scanning = false;
        let networks = [];
        let selectedNetwork = null;
        const canvas = document.getElementById('radarCanvas');
        const ctx = canvas.getContext('2d');
        
        // Set canvas size
        function resizeCanvas() {
            const container = canvas.parentElement;
            const size = Math.min(container.clientWidth, 500);
            canvas.width = size;
            canvas.height = size;
        }
        
        window.addEventListener('resize', resizeCanvas);
        resizeCanvas();

        function drawRadar() {
            const width = canvas.width;
            const height = canvas.height;
            const centerX = width / 2;
            const centerY = height / 2;
            const maxRadius = Math.min(width, height) / 2 - 20;

            // Clear canvas
            ctx.clearRect(0, 0, width, height);

            // Draw radar circles
            ctx.strokeStyle = '#ccc';
            ctx.fillStyle = '#f0f0f0';
            
            for (let i = 1; i <= 4; i++) {
                const radius = (maxRadius / 4) * i;
                ctx.beginPath();
                ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
                ctx.stroke();
                
                // Draw distance labels
                ctx.fillStyle = '#666';
                ctx.font = '12px Arial';
                ctx.textAlign = 'right';
                ctx.fillText(`${i * 25}m`, centerX - radius - 5, centerY);
            }

            // Draw crosshairs
            ctx.beginPath();
            ctx.moveTo(centerX, centerY - maxRadius);
            ctx.lineTo(centerX, centerY + maxRadius);
            ctx.moveTo(centerX - maxRadius, centerY);
            ctx.lineTo(centerX + maxRadius, centerY);
            ctx.stroke();

            // Draw network points
            networks.forEach((network, index) => {
                // Calculate position based on signal strength and a pseudo-random angle
                const distance = Math.min(calculateDistance(network.signal), 100);
                const distanceRatio = distance / 100;
                const radius = maxRadius * distanceRatio;
                
                // Use BSSID to generate a consistent angle for each network
                const angle = hashBSSIDToAngle(network.bssid);
                
                const x = centerX + radius * Math.cos(angle);
                const y = centerY + radius * Math.sin(angle);

                // Draw point
                ctx.beginPath();
                ctx.fillStyle = getSignalColor(network.signalQuality);
                ctx.arc(x, y, network.selected ? 8 : 6, 0, Math.PI * 2);
                ctx.fill();

                // Draw label if selected
                if (network.selected) {
                    ctx.fillStyle = '#000';
                    ctx.font = '12px Arial';
                    ctx.textAlign = 'center';
                    ctx.fillText(network.ssid, x, y - 15);
                }
            });

            // Draw scanning animation
            if (scanning) {
                const now = Date.now();
                const scanAngle = (now / 1000) % (Math.PI * 2);
                
                ctx.beginPath();
                ctx.moveTo(centerX, centerY);
                ctx.arc(centerX, centerY, maxRadius, scanAngle - 0.1, scanAngle);
                ctx.lineTo(centerX, centerY);
                ctx.fillStyle = 'rgba(33, 150, 243, 0.3)';
                ctx.fill();
            }
        }

        function hashBSSIDToAngle(bssid) {
            // Convert BSSID to a consistent angle between 0 and 2π
            let hash = 0;
            for (let i = 0; i < bssid.length; i++) {
                hash = bssid.charCodeAt(i) + ((hash << 5) - hash);
            }
            return (hash % 360) * (Math.PI / 180);
        }

        function getSignalColor(quality) {
            switch(quality) {
                case 'Excellent': return '#4CAF50';
                case 'Good': return '#8BC34A';
                case 'Fair': return '#FFC107';
                default: return '#F44336';
            }
        }

        function calculateDistance(signal) {
            // Simplified distance calculation based on signal strength
            // Using the free space path loss formula
            const signalStrength = Math.abs(signal);
            return Math.min(Math.pow(10, (signalStrength - 30) / 20), 100);
        }

        function getSignalClass(quality) {
            switch(quality) {
                case 'Excellent': return 'excellent';
                case 'Good': return 'good';
                case 'Fair': return 'fair';
                default: return 'poor';
            }
        }

        function formatTimeDiff(timestamp) {
            const diff = Date.now() - timestamp;
            const seconds = Math.floor(diff / 1000);
            if (seconds < 60) return seconds + ' seconds ago';
            const minutes = Math.floor(seconds / 60);
            if (minutes < 60) return minutes + ' minutes ago';
            const hours = Math.floor(minutes / 60);
            return hours + ' hours ago';
        }

        function selectNetwork(bssid) {
            networks.forEach(network => {
                network.selected = network.bssid === bssid;
            });
            drawRadar();
            
            // Update card selection
            document.querySelectorAll('.network-card').forEach(card => {
                card.classList.toggle('selected', card.dataset.bssid === bssid);
            });
        }

        function displayNetworks(networkData) {
            networks = networkData;
            const networkList = document.getElementById('networkList');
            networkList.innerHTML = '';
            
            networks.forEach(network => {
                const card = document.createElement('div');
                card.className = 'network-card';
                card.dataset.bssid = network.bssid;
                card.onclick = () => selectNetwork(network.bssid);
                
                const signalClass = getSignalClass(network.signalQuality);
                
                card.innerHTML = `
                    <div style="display: flex; align-items: center;">
                        <span class="signal-strength ${signalClass}"></span>
                        <strong>${network.ssid}</strong>
                    </div>
                    <p>Signal: ${network.signal} dBm (${network.signalQuality})</p>
                    <p>Channel: ${network.channel} (${network.frequency} MHz)</p>
                    <p>Security: ${network.encryption}</p>
                    <p>Distance: ~${calculateDistance(network.signal).toFixed(1)}m</p>
                `;
                
                networkList.appendChild(card);
            });

            drawRadar();
        }

        async function startScan() {
            if (scanning) return;
            
            scanning = true;
            const scanButton = document.getElementById('scanButton');
            const loading = document.getElementById('loading');
            
            scanButton.disabled = true;
            loading.classList.add('active');
            
            // Start radar animation
            let radarAnimation = setInterval(drawRadar, 50);
            
            try {
                const response = await fetch('/scan');
                const data = await response.json();
                displayNetworks(data.networks);
            } catch (error) {
                console.error('Scan failed:', error);
                alert('Failed to scan networks. Please try again.');
            } finally {
                scanning = false;
                scanButton.disabled = false;
                loading.classList.remove('active');
                clearInterval(radarAnimation);
                drawRadar();
            }
        }

        // Animation loop
        function animate() {
            drawRadar();
            requestAnimationFrame(animate);
        }

        // Start animation and initial scan when page loads
        window.onload = () => {
            animate();
            startScan();
        };
    </script>
</body>
</html>
)rawliteral";

// Helper Functions
String calculateFrequency(int channel) {
    return String(2407 + (channel * 5)) + " MHz";
}

double calculateDistance(int rssi) {
    const double RSSI_AT_1M = -40.0;
    return pow(10.0, (RSSI_AT_1M - rssi) / (10 * 2.0));
}

const char* getSignalQuality(int rssi) {
    if (rssi >= -50) return "Excellent";
    else if (rssi >= -60) return "Good";
    else if (rssi >= -70) return "Fair";
    return "Poor";
}

const char* getSecurityAnalysis(wifi_auth_mode_t encryption) {
    switch (encryption) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
        default: return "Unknown";
    }
}

NetworkInfo* findOrCreateNetwork(const String& ssid, const String& bssid) {
    for (auto& network : networksList) {
        if (network.bssid == bssid) {
            return &network;
        }
    }
    
    NetworkInfo newNetwork;
    newNetwork.ssid = ssid;
    newNetwork.bssid = bssid;
    newNetwork.scanCount = 0;
    newNetwork.firstSeen = millis();
    newNetwork.signalHistory.reserve(MAX_HISTORY_SIZE);
    networksList.push_back(newNetwork);
    return &networksList.back();
}

void updateSignalStability(NetworkInfo* network, int newSignal) {
    network->lastSeen = millis();
    
    if (network->signalHistory.size() >= MAX_HISTORY_SIZE) {
        network->signalHistory.erase(network->signalHistory.begin());
    }
    network->signalHistory.push_back(newSignal);
    
    if (network->scanCount == 0) {
        network->avgSignal = newSignal;
    } else {
        network->avgSignal = (network->avgSignal * network->scanCount + newSignal) / (network->scanCount + 1);
    }
    network->scanCount++;
}

// Web Server Handlers
void handleRoot() {
    server.send(200, "text/html", index_html);
}

void handleScan() {
    // Add debug output
    Serial.println("Starting network scan...");
    
    // Increase JSON document size and add error checking
    StaticJsonDocument<32768> doc;  // Increased size
    JsonArray networks = doc.createNestedArray("networks");

    // Perform sync scan with improved parameters
    int numNetworks = WiFi.scanNetworks(false, true);  // Simplified scan parameters
    Serial.printf("Found %d networks\n", numNetworks);
    
    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            // Debug print for each network
            Serial.printf("Network %d: SSID: %s, RSSI: %d\n", 
                i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            
            String ssid = WiFi.SSID(i);
            String bssid = WiFi.BSSIDstr(i);
            NetworkInfo* network = findOrCreateNetwork(ssid, bssid);
            
            // Update network information
            network->signal = WiFi.RSSI(i);
            network->encryption = getSecurityAnalysis(WiFi.encryptionType(i));
            network->channel = WiFi.channel(i);
            network->isHidden = ssid.length() == 0;
            network->frequency = calculateFrequency(network->channel).toInt();
            
            updateSignalStability(network, network->signal);

            JsonObject networkObj = networks.createNestedObject();
            networkObj["ssid"] = network->isHidden ? "<Hidden Network>" : network->ssid;
            networkObj["bssid"] = network->bssid;
            networkObj["signal"] = network->signal;
            networkObj["signalQuality"] = getSignalQuality(network->signal);
            networkObj["encryption"] = network->encryption;
            networkObj["channel"] = network->channel;
            networkObj["frequency"] = network->frequency;
            networkObj["distance"] = calculateDistance(network->signal);
            networkObj["avgSignal"] = network->avgSignal;
            networkObj["scanCount"] = network->scanCount;
            networkObj["firstSeen"] = network->firstSeen;
            networkObj["lastSeen"] = network->lastSeen;
        }
    } else {
        Serial.println("No networks found");
    }

    String response;
    serializeJson(doc, response);
    
    // Debug print the response
    Serial.println("JSON Response:");
    Serial.println(response);
    
    server.send(200, "application/json", response);

    // Clean up scan
    WiFi.scanDelete();
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    Serial.println("\nInitializing WiFi Radar...");
    
    // Reset WiFi
    WiFi.disconnect(true);
    delay(1000);
    
    // Configure device as AP and station
    WiFi.mode(WIFI_AP_STA);
    
    // Configure Access Point with more specific parameters
    const char* ssid = "WiFi_Radar";
    const char* password = "12345678";  // At least 8 characters for WPA2
    
    if (WiFi.softAP(ssid, password)) {
        Serial.println("Access Point Created Successfully");
    } else {
        Serial.println("Access Point Creation Failed!");
    }
    
    // Setup web server
    server.on("/", HTTP_GET, handleRoot);
    server.on("/scan", HTTP_GET, handleScan);
    server.begin();
    
    Serial.println("WiFi Radar System Started");
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Enable station mode for scanning
    WiFi.enableSTA(true);
}

void loop() {
    server.handleClient();
    
    static unsigned long lastDebugPrint = 0;
    const unsigned long DEBUG_INTERVAL = 5000; // 5 seconds
    
    // Periodic debug output
    if (millis() - lastDebugPrint >= DEBUG_INTERVAL) {
        Serial.printf("System uptime: %lu ms\n", millis());
        Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
        lastDebugPrint = millis();
    }
    
    delay(10);
}