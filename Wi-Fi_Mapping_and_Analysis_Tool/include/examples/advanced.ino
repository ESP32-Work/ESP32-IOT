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

// HTML content with visualization interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Radar System</title>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        :root {
            --primary-color: #00ff00;
            --background-dark: #1a1a1a;
            --panel-background: rgba(0, 0, 0, 0.8);
            --text-color: #0f0;
            --border-color: #333;
        }

        body {
            margin: 0;
            padding: 20px;
            background: var(--background-dark);
            font-family: Arial, sans-serif;
            color: var(--text-color);
        }

        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 20px;
            max-width: 1800px;
            margin: 0 auto;
        }

        .panel {
            background: var(--panel-background);
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
        }

        .radar-container {
            position: relative;
            width: 100%;
            padding-bottom: 100%;
        }

        .radar-display {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            border-radius: 50%;
            border: 2px solid var(--border-color);
            background: radial-gradient(circle, #000 0%, #111 100%);
            overflow: hidden;
        }

        .radar-circles {
            position: absolute;
            width: 100%;
            height: 100%;
        }

        .radar-circle {
            position: absolute;
            border: 1px solid var(--border-color);
            border-radius: 50%;
        }

        .radar-line {
            position: absolute;
            width: 50%;
            height: 2px;
            background: linear-gradient(90deg, 
                rgba(0,255,0,0.5) 0%, 
                rgba(0,255,0,0) 100%);
            transform-origin: 100% 50%;
            top: 50%;
            right: 50%;
            animation: radar-sweep 4s infinite linear;
        }

        .network-point {
            position: absolute;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            transform: translate(-50%, -50%);
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .network-list {
            max-height: 400px;
            overflow-y: auto;
            margin-bottom: 20px;
        }

        .network-item {
            margin-bottom: 15px;
            padding: 10px;
            border-radius: 5px;
            background: rgba(255, 255, 255, 0.1);
        }

        .chart-container {
            position: relative;
            height: 300px;
            margin: 20px 0;
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }

        .stat-card {
            background: rgba(255, 255, 255, 0.1);
            padding: 15px;
            border-radius: 5px;
            text-align: center;
        }

        @keyframes radar-sweep {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }

        .quality-Excellent { background-color: #00ff00; color: #000; }
        .quality-Good { background-color: #90EE90; color: #000; }
        .quality-Fair { background-color: #FFD700; color: #000; }
        .quality-Poor { background-color: #FF4500; color: #fff; }
    </style>
</head>
<body>
    <div class="dashboard">
        <!-- Radar Panel -->
        <div class="panel">
            <h2>WiFi Radar</h2>
            <div class="radar-container">
                <div class="radar-display">
                    <div class="radar-circles">
                        <div class="radar-circle" style="width: 25%; height: 25%; left: 37.5%; top: 37.5%"></div>
                        <div class="radar-circle" style="width: 50%; height: 50%; left: 25%; top: 25%"></div>
                        <div class="radar-circle" style="width: 75%; height: 75%; left: 12.5%; top: 12.5%"></div>
                    </div>
                    <div class="radar-line"></div>
                    <div id="networkPoints"></div>
                </div>
            </div>
        </div>

        <!-- Network List Panel -->
        <div class="panel">
            <h2>Detected Networks</h2>
            <div class="network-list" id="networkList"></div>
        </div>

        <!-- Analytics Panel -->
        <div class="panel">
            <h2>Network Analytics</h2>
            <div class="chart-container">
                <canvas id="channelChart"></canvas>
            </div>
            <div class="stats-grid">
                <div class="stat-card">
                    <div>Total Networks</div>
                    <div id="totalNetworks">0</div>
                </div>
                <div class="stat-card">
                    <div>Average Signal</div>
                    <div id="avgSignal">0 dBm</div>
                </div>
                <div class="stat-card">
                    <div>Most Used Channel</div>
                    <div id="mostUsedChannel">Ch 0</div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let channelChart = null;

        // Initialize the channel utilization chart
        function initializeChart() {
            const ctx = document.getElementById('channelChart').getContext('2d');
            channelChart = new Chart(ctx, {
                type: 'bar',
                data: {
                    labels: Array.from({length: 13}, (_, i) => `Channel ${i + 1}`),
                    datasets: [{
                        label: 'Networks per Channel',
                        data: Array(11).fill(0),
                        backgroundColor: 'rgba(0, 255, 0, 0.5)',
                        borderColor: 'rgba(0, 255, 0, 1)',
                        borderWidth: 1
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            ticks: { color: '#0f0' }
                        },
                        x: {
                            ticks: { color: '#0f0' }
                        }
                    },
                    plugins: {
                        legend: {
                            labels: { color: '#0f0' }
                        }
                    }
                }
            });
        }

        // Update radar visualization
        function updateRadar(networks) {
            const container = document.getElementById('networkPoints');
            container.innerHTML = '';

            networks.forEach(network => {
                const point = document.createElement('div');
                point.className = 'network-point';
                
                const distance = (100 + network.signal) / 100;
                const angle = Math.random() * Math.PI * 2;
                const x = 50 + Math.cos(angle) * (distance * 45);
                const y = 50 + Math.sin(angle) * (distance * 45);
                
                point.style.left = `${x}%`;
                point.style.top = `${y}%`;
                
                const signalStrength = (100 + network.signal) / 100;
                const hue = 120 * signalStrength;
                point.style.backgroundColor = `hsla(${hue}, 100%, 50%, 0.8)`;

                container.appendChild(point);
            });
        }

        // Update network list
        function updateNetworkList(networks) {
            const container = document.getElementById('networkList');
            container.innerHTML = networks.map(network => `
                <div class="network-item">
                    <div style="font-weight: bold;">${network.ssid}</div>
                    <div>Signal: ${network.signal} dBm
                        <span class="quality-${network.signalQuality}">
                            ${network.signalQuality}
                        </span>
                    </div>
                    <div>Channel: ${network.channel}</div>
                    <div>Security: ${network.encryption}</div>
                </div>
            `).join('');
        }

        // Update channel chart
        function updateChannelChart(networks) {
            const channelCounts = Array(13).fill(0);
            networks.forEach(network => {
                if (network.channel >= 1 && network.channel <= 13) {
                    channelCounts[network.channel - 1]++;
                }
            });

            channelChart.data.datasets[0].data = channelCounts;
            channelChart.update();
        }

        // Update statistics
        function updateStats(networks) {
            document.getElementById('totalNetworks').textContent = networks.length;
            
            const avgSignal = networks.reduce((sum, n) => sum + n.signal, 0) / networks.length;
            document.getElementById('avgSignal').textContent = `${avgSignal.toFixed(1)} dBm`;
            
            const channelCounts = {};
            networks.forEach(n => {
                channelCounts[n.channel] = (channelCounts[n.channel] || 0) + 1;
            });
            const mostUsedChannel = Object.entries(channelCounts)
                .sort((a, b) => b[1] - a[1])[0];
            document.getElementById('mostUsedChannel').textContent = 
                `Ch ${mostUsedChannel[0]} (${mostUsedChannel[1]})`;
        }

        // Fetch and update data
        function updateData() {
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    const networks = data.networks || [];
                    updateRadar(networks);
                    updateNetworkList(networks);
                    updateChannelChart(networks);
                    updateStats(networks);
                })
                .catch(console.error);
        }

        // Initialize
        document.addEventListener('DOMContentLoaded', () => {
            initializeChart();
            updateData();
            setInterval(updateData, 10000); // Update every 10 seconds
        });
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
    StaticJsonDocument<16384> doc;
    JsonArray networks = doc.createNestedArray("networks");

    int numNetworks = WiFi.scanNetworks(false, true, false, 300);

    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            String ssid = WiFi.SSID(i);
            String bssid = WiFi.BSSIDstr(i);
            int signal = WiFi.RSSI(i);
            int channel = WiFi.channel(i);
            wifi_auth_mode_t encryption = WiFi.encryptionType(i);
            
            // Create or find the network info object
            NetworkInfo* network = findOrCreateNetwork(ssid, bssid);

            // Update signal strength stability and frequency
            updateSignalStability(network, signal);
            network->frequency = calculateFrequency(channel).toInt();

            // Set network properties
            network->ssid = ssid;
            network->signal = signal;
            network->encryption = getSecurityAnalysis(encryption);
            network->channel = channel;
            network->isHidden = ssid.isEmpty();  // Check if SSID is empty, which indicates a hidden network
            network->lastSeen = millis();

            // Create JSON object for each network
            JsonObject networkObj = networks.createNestedObject();
            networkObj["ssid"] = network->ssid;
            networkObj["signal"] = network->signal;
            networkObj["avgSignal"] = network->avgSignal;
            networkObj["encryption"] = network->encryption;
            networkObj["channel"] = network->channel;
            networkObj["isHidden"] = network->isHidden;
            networkObj["bssid"] = network->bssid;
            networkObj["frequency"] = network->frequency;
            networkObj["signalQuality"] = getSignalQuality(network->signal);
        }
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}


void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    delay(100);
    // Start ESP32 as an open access point
    WiFi.softAP("radar");  // Open AP with SSID "radar"

    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point \"radar\" started. IP address: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/scan", handleScan);
    server.begin();
    Serial.println("WiFi Radar System initialized");
}

void loop() {
    server.handleClient();
}