#include <Bonezegei_DHT22.h>
#include <WiFi.h>
#include <WebServer.h>

Bonezegei_DHT22 dht1(21);
Bonezegei_DHT22 dht2(22);

// WiFi Configuration
const char* ssid = "DHT22_Access_Point";
const char* password = "password123";

WebServer server(80);

// Global variables to store the latest data
float g_temp1, g_temp2;
int g_humi1, g_humi2;
unsigned long lastSensorRead = 0;

// This html will be served to the user as they access the IP Address when connected to the Access Point
void handleRoot() {
  String html = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <meta http-equiv='refresh' content='5'>
        <style>
          body { font-family:Arial; text-align:center; background:#1a1a1a; color:#eee; }
          .card { background:#2d2d2d; margin:20px auto; padding:20px; width:80%; max-width:300px; border-radius:10px; border: 1px solid #444; }
          .temp { color: #ffa500; font-size: 1.5rem; }
          .humi { color: #00bfff; font-size: 1.5rem; }
        </style>
    </head>
    <body>
      <h1>System Monitor</h1>
  )=====";

  html += "<div class='card'><h2>Sensor #1</h2>";
  html += "<p class='temp'>" + String(g_temp1) + " °C</p>";
  html += "<p class='humi'>" + String(g_humi1) + " %</p></div>";

  html += "<div class='card'><h2>Sensor #2</h2>";
  html += "<p class='temp'>" + String(g_temp2) + " °C</p>";
  html += "<p class='humi'>" + String(g_humi2) + " %</p></div>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht1.begin();
  dht2.begin();

  // Set up Access Point
  Serial.println("Configuring Access Point...");
  WiFi.softAP(ssid, password);
 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Define Server Routes
  server.on("/", handleRoot);
 
  // Start Server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Read sensors every 2 seconds independently of the web server
  if (millis() - lastSensorRead > 2000) {
    if (dht1.getData()) {
      g_temp1 = dht1.getTemperature();
      g_humi1 = dht1.getHumidity();
    }
    if (dht2.getData()) {
      g_temp2 = dht2.getTemperature();
      g_humi2 = dht2.getHumidity();
    }
    lastSensorRead = millis();
  }
}