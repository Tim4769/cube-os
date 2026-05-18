#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* apName = "CubeOS-Setup";
const char* apPassword = "12345678";
const unsigned long bootStabilizeDelayMs = 3000;
const unsigned long wifiConnectTimeoutMs = 15000;
const unsigned long wifiConnectCheckIntervalMs = 500;

WebServer server(80);
bool webServerStarted = false;

void startWebServerOnce();
void handleRoot();
void handleConnect();
void handleFavicon();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  delay(bootStabilizeDelayMs);
  Serial.print("Reset reason: ");
  Serial.println(esp_reset_reason());

  WiFi.mode(WIFI_AP_STA);

  bool ok = WiFi.softAP(apName, apPassword);

  if (!ok) {
    Serial.println("Failed to start AP");
    return;
  }

  Serial.println("ESP32 Wi-Fi started");
  Serial.print("Network name: ");
  Serial.println(apName);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  startWebServerOnce();
}

void startWebServerOnce() {
  if (webServerStarted) {
    return;
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/favicon.ico", HTTP_GET, handleFavicon);
  server.onNotFound(handleNotFound);
  server.begin();
  webServerStarted = true;

  Serial.println("Web server started");
}

void loop() {
  if (webServerStarted) {
    server.handleClient();
  }
}

void handleRoot() {
  Serial.println("Browser requested /");

  server.send(200, "text/html",
    "<h1>ESP32 Access Point Works</h1>"
    "<p>Enter the SSID and password of your home network in the fields below.</p>"
    "<form action='/connect' method='post'>"
    "<label for='ssid'>SSID:</label>"
    "<input type='text' id='ssid' name='ssid'><br><br>"
    "<label for='password'>Password:</label>"
    "<input type='password' id='password' name='password'><br><br>"
    "<input type='submit' value='Connect'>"
    "</form>");
}

void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  if (ssid.length() == 0) {
    server.send(400, "text/html",
      "<h1>Missing SSID</h1>"
      "<p>Please go back and enter a Wi-Fi network name.</p>");
    return;
  }

  Serial.print("Connecting to Wi-Fi network: ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < wifiConnectTimeoutMs) {
    delay(wifiConnectCheckIntervalMs);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. Station IP address: ");
    Serial.println(WiFi.localIP());

    server.send(200, "text/html",
      "<h1>Connected</h1>"
      "<p>The ESP32 connected to the Wi-Fi network.</p>");
    return;
  }

  Serial.println("Wi-Fi connection failed");

  server.send(200, "text/html",
    "<h1>Connection failed</h1>"
    "<p>Please check the SSID and password, then try again.</p>"
    "<a href='/'>Back</a>");
}

void handleFavicon() {
  server.send(204);
}

void handleNotFound() {
  Serial.print("No handler for: ");
  Serial.println(server.uri());

  server.send(404, "text/plain", "Not found");
}
