Aap ek expert Embedded Systems aur PlatformIO developer hain. Mujhe mere ESP32 DevKit V1 (ESP32 WROOM-32) ke liye ek custom firmware compile karke .bin file generate karni hai. 

Main aapko neeche C++ ka code de raha hoon. Aapko ye karna hai:
1. Mere computer par ek naya folder banayein (jaise "Ghost_Firmware").
2. Us folder mein PlatformIO ka structure set karein (platformio.ini aur src/main.cpp).
3. platformio.ini mein board "esp32dev" aur framework "arduino" set karein.
4. Neeche diya gaya code src/main.cpp mein save karein.
5. Terminal mein `pio run` command chala kar isko compile karein.
6. Compile hone ke baad, `.pio/build/esp32dev/firmware.bin` file ko copy karke mujhe dein, taake main usko ESP32 par flash kar sakoon. 
Agar aapke system par PlatformIO install nahi hai, toh pehle usko install karein. Koi error nahi aana chahiye.

Yahan mera C++ code hai:

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

const char* ssid = "Ghost";
const char* password = "ghost920";
WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Ghost Firmware</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #121212; color: #ffffff; margin: 0; padding: 20px; text-align: center; }
    h1 { color: #00ffcc; text-shadow: 0 0 10px #00ffcc; }
    .container { max-width: 600px; margin: auto; }
    .card { background: #1e1e1e; padding: 20px; margin: 15px 0; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.5); }
    button { background: #00ffcc; color: #000; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; font-weight: bold; margin: 5px; transition: 0.3s; }
    button:hover { background: #00ccaa; }
    .danger { background: #ff4444; color: white; }
    .danger:hover { background: #cc0000; }
    .status { margin-top: 10px; font-weight: bold; color: #ffaa00; }
    #network-list { text-align: left; margin-top: 10px; }
    .net-item { background: #2a2a2a; padding: 10px; margin: 5px 0; border-radius: 5px; display: flex; justify-content: space-between; }
  </style>
</head>
<body>
  <div class="container">
    <h1>👻 Ghost Firmware</h1>
    <div class="card">
      <h3>WiFi Scanner</h3>
      <button onclick="scanWiFi()">Scan Networks</button>
      <div id="network-list"></div>
    </div>
    <div class="card">
      <h3>Attacks</h3>
      <button class="danger" onclick="startAttack('deauth')">WiFi Deauth</button>
      <button class="danger" onclick="startAttack('eviltwin')">Evil Twin</button>
      <button class="danger" onclick="startAttack('blespam')">Apple/BLE Spam</button>
      <div class="status" id="attack-status">Status: Idle</div>
    </div>
  </div>
  <script>
    function scanWiFi() {
      document.getElementById('network-list').innerHTML = 'Scanning...';
      fetch('/scan').then(res => res.json()).then(data => {
        let html = '';
        data.forEach(net => {
          html += `<div class="net-item"><span>${net.ssid}</span><span>${net.rssi} dBm</span></div>`;
        });
        document.getElementById('network-list').innerHTML = html;
      });
    }
    function startAttack(type) {
      document.getElementById('attack-status').innerText = 'Status: Running ' + type;
      fetch('/attack?type=' + type);
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
  }

  WiFi.softAP(ssid, password);
  Serial.println("Ghost AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/scan", []() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
      if (i > 0) json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/attack", []() {
    if (server.hasArg("type")) {
      String type = server.arg("type");
      if (type == "deauth") {
        Serial.println("Starting WiFi Deauth...");
      } else if (type == "eviltwin") {
        Serial.println("Starting Evil Twin...");
      } else if (type == "blespam") {
        Serial.println("Starting BLE Spam...");
      }
    }
    server.send(200, "text/plain", "Attack Started");
  });

  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  server.handleClient();
}
