#include <Bonezegei_DHT22.h>
#include <WiFi.h>
#include <WebServer.h>

Bonezegei_DHT22 dht1(21);
Bonezegei_DHT22 dht2(22);

// WiFi Config
// 192.168.4.1
const char* ssid = "DHT22_Pump";
const char* password = "password123";
const int WaterPump = 19;

WebServer server(80);

// Data Variables
float g_temp1, g_temp2, avgTemp;
float constant = 25.00; // used for calibration
bool manualMode = false;
bool manualPumpState = false;

// Recommended max value, anything below and the pump should not be running
const float TEMP_THRESHOLD = 27.0; 

unsigned long lastSensorRead = 0;

void handleRoot() {
  String html = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <style>
          body { font-family: 'Segoe UI', sans-serif; text-align: center; background: #121212; color: #e0e0e0; padding: 20px; }
          .container { max-width: 400px; margin: auto; }
          .card { background: #1e1e1e; padding: 15px; border-radius: 15px; box-shadow: 0 4px 10px rgba(0,0,0,0.5); margin-bottom: 20px; border: 1px solid #333; }
          h2 { color: #4CAF50; margin-bottom: 5px; }
          .val { font-size: 1.8rem; font-weight: bold; color: #00d4ff; }
          .btn { display: inline-block; padding: 12px 24px; margin: 10px; border-radius: 25px; border: none; font-weight: bold; cursor: pointer; text-decoration: none; }
          .btn-on { background: #4CAF50; color: white; }
          .btn-off { background: #f44336; color: white; }
          .btn-mode { background: #555; color: white; }
          .status { font-style: italic; color: #bbb; }
        </style>
    </head>
    <body>
      <div class='container'>
        <h1>🥬 Lettuce Monitor</h1>
        
        <div class='card'>
          <p>Sensor 1: <span class='val'>)=====";
  html += String(g_temp1, 1) + "°C</span></p>";
  html += "<p>Sensor 2: <span class='val'>" + String(g_temp2, 1) + "°C</span></p>";
  html += "<hr style='border:0.5px solid #333'>";
  html += "<h2>Average: " + String(avgTemp, 1) + "°C</h2>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h3>Pump Control</h3>";
  html += "<p class='status'>Mode: " + String(manualMode ? "MANUAL" : "AUTOMATIC") + "</p>";
  
  // Toggle Manual Mode
  html += "<a href='/toggleMode' class='btn btn-mode'>" + String(manualMode ? "Switch to Auto" : "Switch to Manual") + "</a>";

  if (manualMode) {
    html += "<br><a href='/pumpOn' class='btn btn-on'>Turn ON</a>";
    html += "<a href='/pumpOff' class='btn btn-off'>Turn OFF</a>";
  }

  html += "<p>Pump is currently: <b>" + String(digitalRead(WaterPump) ? "RUNNING" : "STOPPED") + "</b></p>";
  html += "</div>";
  
  html += "<p><a href='/' style='color:#777'>Refresh Data</a></p>";
  html += "</div></body></html>";

  // Using UTF-8 header fixes the weird 'A' before the degree symbol
  server.send(200, "text/html; charset=utf-8", html);
}

// Route Handlers
void handleToggleMode() { manualMode = !manualMode; server.sendHeader("Location", "/"); server.send(303); }
void handlePumpOn() { if(manualMode) digitalWrite(WaterPump, HIGH); server.sendHeader("Location", "/"); server.send(303); }
void handlePumpOff() { if(manualMode) digitalWrite(WaterPump, LOW); server.sendHeader("Location", "/"); server.send(303); }

void setup() {
  pinMode(WaterPump, OUTPUT);
  digitalWrite(WaterPump, LOW);
  
  dht1.begin();
  dht2.begin();

  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/toggleMode", handleToggleMode);
  server.on("/pumpOn", handlePumpOn);
  server.on("/pumpOff", handlePumpOff);
  server.begin();
}

void loop() {
  server.handleClient();

  if (millis() - lastSensorRead > 2000) {
    if (dht1.getData() && dht2.getData()) {
      g_temp1 = dht1.getTemperature();
      g_temp2 = dht2.getTemperature();
      g_temp1 += constant;
      g_temp2 += constant;
      avgTemp = (g_temp1 + g_temp2) / 2.0;

      // Automatic Logic (Only runs if manualMode is false)
      if (!manualMode) {
        if (avgTemp > TEMP_THRESHOLD) {
          digitalWrite(WaterPump, HIGH); // Cool down/Circulate
        } else {
          digitalWrite(WaterPump, LOW);
        }
      }
    }
    lastSensorRead = millis();
  }
}