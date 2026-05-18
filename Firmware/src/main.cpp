#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

// This firmware has two main modes:
// 1. Setup mode: the ESP32 creates its own Wi-Fi network so the user can enter Wi-Fi details.
// 2. Normal mode: the ESP32 connects to the saved Wi-Fi and serves the dashboard.

// Setup access point details. These are only used when the ESP32 needs Wi-Fi setup.
const char* apName = "CubeOS-Setup";
const char* apPassword = "12345678";

// mDNS lets a browser open http://cubeos.local instead of typing the numeric IP address.
const char* mdnsName = "cubeos";

// Names used inside ESP32 NVS flash storage for the saved Wi-Fi credentials.
const char* wifiNamespace = "wifi";
const char* ssidKey = "ssid";
const char* passwordKey = "password";
const char* configuredKey = "configured";

// Timing values are in milliseconds.
const unsigned long bootStabilizeDelayMs = 3000;
const unsigned long wifiConnectTimeoutMs = 15000;
const unsigned long wifiConnectCheckIntervalMs = 500;
const unsigned long mdnsRetryIntervalMs = 5000;

// The web server listens on port 80, the normal HTTP port.
WebServer server(80);

// Preferences is Arduino's wrapper around ESP32 NVS flash storage.
Preferences preferences;

// These variables remember the current runtime state of the firmware.
bool webServerStarted = false;
bool setupModeActive = false;
bool mdnsStarted = false;
unsigned long lastMdnsAttemptAt = 0;

// Function declarations. They let functions call each other before their full code appears.
void startSetupMode();
void startNormalMode();
bool loadWiFiCredentials(String& ssid, String& password);
bool connectToWiFi(const String& ssid, const String& password);
bool startSetupAccessPoint();
void startWebServerOnce();
bool startMdns();
void maintainMdns();
bool saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();
void handleRoot();
void handleSetupPage();
void handleConnect();
void handleDebug();
void handleFavicon();
void handleNotFound();
String htmlEscape(const String& text);
int wifiSignalPercent(int rssi);
const char* wifiSignalLabel(int rssi);
const char* wifiModeLabel(wifi_mode_t mode);
void appendPageStart(String& page, const char* title);
void appendPageEnd(String& page);
void sendMessagePage(int code, const char* title, const char* message, const char* linkHref, const char* linkText);

// setup() runs once every time the ESP32 boots or restarts.
void setup() {
  // Step 1: Start the Serial Monitor so we can see debug messages on the computer.
  Serial.begin(115200);

  // Step 2: Wait a little so the ESP32 and Serial Monitor have time to settle after boot.
  delay(bootStabilizeDelayMs);

  // Step 3: Print why the ESP32 restarted. This is useful when debugging.
  Serial.print("Reset reason: ");
  Serial.println(esp_reset_reason());

  // Step 4: Create empty variables where saved Wi-Fi credentials can be loaded.
  String savedSsid;
  String savedPassword;

  // Step 5: Try to load Wi-Fi details that were saved earlier in NVS.
  if (loadWiFiCredentials(savedSsid, savedPassword)) {
    Serial.println("Trying saved Wi-Fi credentials...");

    // Step 6: If saved Wi-Fi works, go straight to normal dashboard mode.
    if (connectToWiFi(savedSsid, savedPassword)) {
      startNormalMode();
      return;
    }

    // Step 7: If saved Wi-Fi fails, do not erase it. Start setup mode instead.
    Serial.println("Saved Wi-Fi failed. Starting setup mode without deleting saved credentials.");
  } else {
    // Step 8: If there are no saved credentials at all, start setup mode.
    Serial.println("No saved Wi-Fi credentials. Starting setup mode.");
  }

  // Step 9: Setup mode lets a user enter Wi-Fi details through a browser.
  startSetupMode();
}

// loop() runs forever after setup(). It must stay quick so the web server can respond.
void loop() {
  // Step 1: Keep mDNS alive so cubeos.local can keep working.
  maintainMdns();

  // Step 2: If the web server is running, check for browser requests.
  if (webServerStarted) {
    server.handleClient();
  }
}

// Setup mode starts a temporary ESP32 Wi-Fi network and shows the Wi-Fi setup form.
void startSetupMode() {
  // Step 1: Remember that the ESP32 is currently in setup mode.
  setupModeActive = true;

  // Step 2: Start the ESP32's own setup Wi-Fi network.
  if (!startSetupAccessPoint()) {
    return;
  }

  // Step 3: Start the web server so the setup page can be opened.
  startWebServerOnce();
}

// Normal mode is used after the ESP32 is connected to the saved home Wi-Fi.
void startNormalMode() {
  // Step 1: Remember that setup mode is finished.
  setupModeActive = false;

  // Step 2: Print the IP address. You can open this IP in a browser.
  Serial.println("Normal mode started");
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());

  // Step 3: Start the dashboard web server.
  startWebServerOnce();

  // Step 4: Start mDNS so http://cubeos.local can work on the same Wi-Fi.
  startMdns();
}

// Reads saved Wi-Fi credentials from NVS flash storage.
// The & symbols mean this function fills in the caller's ssid and password variables.
bool loadWiFiCredentials(String& ssid, String& password) {
  // Step 1: Open the "wifi" namespace in read-only mode.
  if (!preferences.begin(wifiNamespace, true)) {
    Serial.println("Failed to open NVS for reading.");
    return false;
  }

  // Step 2: Read the configured flag to see if Wi-Fi was saved before.
  bool configured = preferences.getBool(configuredKey, false);

  // Step 3: If configured is false, there are no saved credentials to use.
  if (!configured) {
    preferences.end();
    return false;
  }

  // Step 4: Read the saved SSID and password from NVS.
  ssid = preferences.getString(ssidKey, "");
  password = preferences.getString(passwordKey, "");

  // Step 5: Close NVS after reading.
  preferences.end();

  // Step 6: A saved credential set is not useful if the SSID is empty.
  if (ssid.length() == 0) {
    Serial.println("Saved Wi-Fi credentials are invalid: SSID is empty.");
    return false;
  }

  // Step 7: Return true to tell setup() that credentials were loaded.
  Serial.println("Wi-Fi credentials loaded from NVS.");
  Serial.print("Saved SSID: ");
  Serial.println(ssid);
  return true;
}

// Tries to connect to a Wi-Fi network. Returns true only when the connection succeeds.
bool connectToWiFi(const String& ssid, const String& password) {
  // Step 1: Choose the correct Wi-Fi mode.
  // WIFI_STA means the ESP32 connects to a router like a normal Wi-Fi device.
  // WIFI_AP_STA means it can keep setup mode active while also trying router Wi-Fi.
  wifi_mode_t currentMode = WiFi.getMode();
  if (currentMode == WIFI_AP) {
    WiFi.mode(WIFI_AP_STA);
  } else if (currentMode != WIFI_AP_STA) {
    WiFi.mode(WIFI_STA);
  }

  // Step 2: Disconnect from any old Wi-Fi attempt without erasing saved router data.
  WiFi.disconnect(false);

  // Step 3: Set the network hostname before connecting.
  WiFi.setHostname(mdnsName);

  // Step 4: Wait briefly so the Wi-Fi mode change can settle.
  delay(100);

  // Step 5: Start connecting to the requested Wi-Fi network.
  WiFi.begin(ssid.c_str(), password.c_str());

  // Step 6: Wait for Wi-Fi, but give up after wifiConnectTimeoutMs so the ESP32 does not hang forever.
  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < wifiConnectTimeoutMs) {
    delay(wifiConnectCheckIntervalMs);
    Serial.print(".");
  }
  Serial.println();

  // Step 7: Return true if the ESP32 is connected.
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  // Step 8: If connection failed, disconnect and return false.
  WiFi.disconnect(false);
  return false;
}

// Setup AP is started only when there are no credentials or saved Wi-Fi fails.
bool startSetupAccessPoint() {
  // Step 1: Make sure AP mode is enabled so the ESP32 can create its own Wi-Fi network.
  wifi_mode_t currentMode = WiFi.getMode();
  if (currentMode == WIFI_STA) {
    WiFi.mode(WIFI_AP_STA);
  } else if (currentMode != WIFI_AP && currentMode != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
  }

  // Step 2: Start the setup network with the configured name and password.
  bool ok = WiFi.softAP(apName, apPassword);

  // Step 3: Stop here if the setup network could not start.
  if (!ok) {
    Serial.println("Failed to start setup AP");
    return false;
  }

  // Step 4: Print the setup network information for the Serial Monitor.
  Serial.println("Setup access point started");
  Serial.print("Network name: ");
  Serial.println(apName);
  Serial.print("Setup URL: http://");
  Serial.println(WiFi.softAPIP());

  // Step 5: Return true so startSetupMode() knows the AP is ready.
  return true;
}

// Registers all HTTP routes and starts the web server once.
void startWebServerOnce() {
  // Step 1: If the server already started, do nothing.
  if (webServerStarted) {
    return;
  }

  // Step 2: Tell the web server which function handles each URL.
  server.on("/", HTTP_GET, handleRoot);
  server.on("/debug", HTTP_GET, handleDebug);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/favicon.ico", HTTP_GET, handleFavicon);
  server.onNotFound(handleNotFound);

  // Step 3: Start listening for browser requests.
  server.begin();

  // Step 4: Remember that the server is now running.
  webServerStarted = true;

  Serial.println("Web server started");
}

// Starts mDNS so users can open http://cubeos.local on the same Wi-Fi network.
bool startMdns() {
  // Step 1: mDNS only works after the ESP32 has joined a Wi-Fi network.
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  // Step 2: Remember when this attempt happened, so retry timing works.
  lastMdnsAttemptAt = millis();

  // Step 3: Set the ESP32 hostname to "cubeos".
  WiFi.setHostname(mdnsName);

  // Step 4: Start the mDNS responder. This creates cubeos.local.
  if (!MDNS.begin(mdnsName)) {
    Serial.println("Failed to start mDNS. Will retry.");
    return false;
  }

  // Step 5: Advertise that this device has an HTTP web server on port 80.
  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "path", "/");

  // Step 6: Remember that mDNS is running and print the local URL.
  mdnsStarted = true;

  Serial.print("mDNS address: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
  return true;
}

// Keeps mDNS healthy. If mDNS failed earlier, this retries every few seconds.
void maintainMdns() {
  // Step 1: If Wi-Fi is disconnected, mDNS cannot work.
  if (WiFi.status() != WL_CONNECTED) {
    // Step 2: If mDNS was running before, stop it and mark it as stopped.
    if (mdnsStarted) {
      MDNS.end();
      mdnsStarted = false;
    }
    return;
  }

  // Step 3: If mDNS is not running, retry every mdnsRetryIntervalMs.
  if (!mdnsStarted && millis() - lastMdnsAttemptAt >= mdnsRetryIntervalMs) {
    startMdns();
  }
}

// Saves Wi-Fi credentials to NVS only after a successful connection test.
bool saveWiFiCredentials(const String& ssid, const String& password) {
  // Step 1: Open the "wifi" namespace in read/write mode.
  if (!preferences.begin(wifiNamespace, false)) {
    Serial.println("Failed to open NVS for writing.");
    return false;
  }

  // Step 2: Save the SSID, password, and configured flag.
  preferences.putString(ssidKey, ssid);
  preferences.putString(passwordKey, password);
  preferences.putBool(configuredKey, true);

  // Step 3: Close NVS after writing.
  preferences.end();

  // Step 4: Return true so the caller knows the save worked.
  Serial.println("Wi-Fi credentials saved to NVS.");
  return true;
}

// Not called by the normal dashboard. This is here for a future factory-reset button.
void clearWiFiCredentials() {
  // Step 1: Open the "wifi" namespace in read/write mode.
  if (!preferences.begin(wifiNamespace, false)) {
    Serial.println("Failed to open NVS for writing.");
    return;
  }

  // Step 2: Remove each Wi-Fi-related key.
  preferences.remove(ssidKey);
  preferences.remove(passwordKey);
  preferences.remove(configuredKey);

  // Step 3: Close NVS after removing the data.
  preferences.end();

  Serial.println("Wi-Fi credentials cleared.");
}

// Handles GET /. In setup mode it shows the Wi-Fi form; otherwise it shows the dashboard.
void handleRoot() {
  // Step 1: Log the browser request in the Serial Monitor.
  Serial.println("Browser requested /");

  // Step 2: If the ESP32 is not online yet, show the setup form instead of the dashboard.
  if (setupModeActive || WiFi.status() != WL_CONNECTED) {
    handleSetupPage();
    return;
  }

  // Step 3: Create a String to hold the complete HTML page.
  String page;
  page.reserve(4800);

  // Step 4: Read the real Wi-Fi signal strength and convert it to a display percentage.
  int rssi = WiFi.RSSI();
  int signalPercent = wifiSignalPercent(rssi);

  // Step 5: Build the shared page header and the top dashboard title.
  appendPageStart(page, "Cube OS Dashboard");
  page += "<header class='topbar'>";
  page += "<div><p class='eyebrow'>Cube OS</p><h1>Device dashboard</h1></div>";
  page += "<span class='status-pill online'>Online</span>";
  page += "</header>";

  // Step 6: Add the top summary row: Wi-Fi name, IP address, and mDNS address.
  page += "<section class='summary'>";
  page += "<div><span class='label'>Network</span><strong>";
  page += htmlEscape(WiFi.SSID());
  page += "</strong></div>";
  page += "<div><span class='label'>Access URL</span><strong>http://";
  page += WiFi.localIP().toString();
  page += "</strong></div>";
  page += "<div><span class='label'>mDNS URL</span><strong>http://";
  page += mdnsName;
  page += ".local</strong></div>";
  page += "</section>";

  // Step 7: Add the connection panel with signal percentage and RSSI.
  page += "<main class='grid'>";
  page += "<section class='panel primary'>";
  page += "<span class='panel-title'>Connection</span>";
  page += "<div class='metric-row'><div><span class='metric'>";
  page += String(signalPercent);
  page += "%</span><span class='metric-label'>";
  page += wifiSignalLabel(rssi);
  page += " signal</span></div><span class='status-dot'></span></div>";
  page += "<div class='meter'><span style='width:";
  page += String(signalPercent);
  page += "%'></span></div>";
  page += "<div class='detail-list'>";
  page += "<p><span>RSSI</span><strong>";
  page += String(rssi);
  page += " dBm</strong></p>";
  page += "<p><span>Status</span><strong>Connected</strong></p>";
  page += "</div></section>";

  // Step 8: Add the address panel with IP, mDNS status, and Wi-Fi mode.
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Addresses</span>";
  page += "<div class='detail-list'>";
  page += "<p><span>IP address</span><strong>";
  page += WiFi.localIP().toString();
  page += "</strong></p>";
  page += "<p><span>mDNS</span><strong>";
  page += mdnsName;
  page += ".local</strong></p>";
  page += "<p><span>mDNS status</span><strong>";
  page += mdnsStarted ? "Started" : "Not started";
  page += "</strong></p>";
  page += "<p><span>Mode</span><strong>";
  page += wifiModeLabel(WiFi.getMode());
  page += "</strong></p>";
  page += "</div></section>";

  // Step 9: Add the device panel with hostname and uptime.
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Device</span>";
  page += "<div class='detail-list'>";
  page += "<p><span>Hostname</span><strong>";
  page += mdnsName;
  page += "</strong></p>";
  page += "<p><span>Uptime</span><strong>";
  page += String(millis() / 1000);
  page += " sec</strong></p>";
  page += "</div></section>";
  page += "</main>";

  // Step 10: Add action buttons and close the HTML page.
  page += "<section class='actions'>";
  page += "<a class='button secondary' href='/debug'>Debug</a>";
  page += "<a class='button secondary' href='/'>Refresh</a>";
  page += "</section>";
  appendPageEnd(page);

  // Step 11: Send the completed dashboard page to the browser.
  server.send(200, "text/html", page);
}

// Shows the first-time Wi-Fi setup form.
// This is only reachable from / while setupModeActive is true.
void handleSetupPage() {
  // Step 1: Log that the setup page was requested.
  Serial.println("Browser requested Wi-Fi setup page");

  // Step 2: Create a String to hold the complete setup page.
  String page;
  page.reserve(3600);

  // Step 3: Add the shared page header and setup title.
  appendPageStart(page, "Cube OS Wi-Fi Setup");
  page += "<header class='topbar'>";
  page += "<div><p class='eyebrow'>Cube OS</p><h1>Wi-Fi setup</h1></div>";
  page += setupModeActive ? "<span class='status-pill setup'>Setup mode</span>" : "<span class='status-pill online'>Online</span>";
  page += "</header>";

  // Step 4: Add the Wi-Fi form. The form sends data to POST /connect.
  page += "<main class='setup-grid'>";
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Network credentials</span>";
  page += "<form action='/connect' method='post'>";
  page += "<label for='ssid'>SSID</label>";
  page += "<input type='text' id='ssid' name='ssid' autocomplete='off' required>";
  page += "<label for='password'>Password</label>";
  page += "<input type='password' id='password' name='password' autocomplete='current-password'>";
  page += "<button type='submit'>Connect and save</button>";
  page += "</form>";
  page += "</section>";

  // Step 5: Add a small panel showing the setup network and setup URL.
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Current access</span>";
  page += "<div class='detail-list'>";
  page += "<p><span>Setup network</span><strong>";
  page += apName;
  page += "</strong></p>";
  page += "<p><span>Setup URL</span><strong>http://";
  page += WiFi.softAPIP().toString();
  page += "</strong></p>";
  page += "<p><span>Saved data</span><strong>NVS protected from failed logins</strong></p>";
  page += "</div>";
  page += "</section>";
  page += "</main>";
  appendPageEnd(page);

  // Step 6: Send the setup page to the browser.
  server.send(200, "text/html", page);
}

// Receives the setup form. It tests Wi-Fi first, then saves credentials only if the test works.
void handleConnect() {
  // Step 1: Block Wi-Fi changes once the dashboard is already online.
  if (!setupModeActive) {
    sendMessagePage(403, "Wi-Fi changes disabled", "This device is already online. Wi-Fi setup is only available during setup mode.", "/", "Open dashboard");
    return;
  }

  // Step 2: Read the SSID and password that the user typed into the form.
  String newSsid = server.arg("ssid");
  String newPassword = server.arg("password");

  // Step 3: SSID is required. Password can be empty for open networks.
  if (newSsid.length() == 0) {
    sendMessagePage(400, "Missing SSID", "Please go back and enter a Wi-Fi network name.", "/", "Back to setup");
    return;
  }

  // Step 4: Print which network is being tested.
  Serial.print("Connecting to Wi-Fi network: ");
  Serial.println(newSsid);

  // Step 5: Test the new Wi-Fi before saving it.
  // The old saved credentials stay untouched if this connection attempt fails.
  if (connectToWiFi(newSsid, newPassword)) {
    Serial.print("Connected. Station IP address: ");
    Serial.println(WiFi.localIP());

    // Step 6: Save the new credentials only after the connection succeeds.
    if (!saveWiFiCredentials(newSsid, newPassword)) {
      sendMessagePage(500, "Save failed", "The ESP32 connected to Wi-Fi, but could not save the credentials.", "/", "Open dashboard");
      return;
    }

    // Step 7: Tell the user it worked, then restart so boot logic uses the saved credentials.
    sendMessagePage(200, "Connected", "Wi-Fi credentials saved. Restarting ESP32...", "/", "Open dashboard");

    delay(1500);
    ESP.restart();
    return;
  }

  // Step 8: If connection failed, show an error and do not save the bad credentials.
  Serial.println("Wi-Fi connection failed");

  sendMessagePage(200, "Connection failed", "Please check the SSID and password. The new details were not saved, and old settings were not erased.", "/", "Try again");
}

// Shows network details that are useful when cubeos.local does not resolve.
void handleDebug() {
  // Step 1: Log the debug page request.
  Serial.println("Browser requested /debug");

  // Step 2: Create a String to hold the complete debug page.
  String page;
  page.reserve(4200);

  // Step 3: Add the shared page header and mDNS status badge.
  appendPageStart(page, "Cube OS Debug");
  page += "<header class='topbar'>";
  page += "<div><p class='eyebrow'>Cube OS</p><h1>mDNS debug</h1></div>";
  page += mdnsStarted ? "<span class='status-pill online'>mDNS started</span>" : "<span class='status-pill setup'>mDNS not started</span>";
  page += "</header>";

  // Step 4: Add current Wi-Fi and hostname information.
  page += "<main class='setup-grid'>";
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Network state</span>";
  page += "<div class='detail-list'>";
  page += "<p><span>Wi-Fi status</span><strong>";
  page += (WiFi.status() == WL_CONNECTED) ? "Connected" : "Not connected";
  page += "</strong></p>";
  page += "<p><span>Wi-Fi mode</span><strong>";
  page += wifiModeLabel(WiFi.getMode());
  page += "</strong></p>";
  page += "<p><span>SSID</span><strong>";
  page += htmlEscape(WiFi.SSID());
  page += "</strong></p>";
  page += "<p><span>Hostname</span><strong>";
  page += mdnsName;
  page += "</strong></p>";
  page += "<p><span>mDNS URL</span><strong>http://";
  page += mdnsName;
  page += ".local</strong></p>";
  page += "</div></section>";

  // Step 5: Add IP, gateway, subnet, DNS, and MAC address information.
  page += "<section class='panel'>";
  page += "<span class='panel-title'>Addresses</span>";
  page += "<div class='detail-list'>";
  page += "<p><span>IP</span><strong>";
  page += WiFi.localIP().toString();
  page += "</strong></p>";
  page += "<p><span>Gateway</span><strong>";
  page += WiFi.gatewayIP().toString();
  page += "</strong></p>";
  page += "<p><span>Subnet</span><strong>";
  page += WiFi.subnetMask().toString();
  page += "</strong></p>";
  page += "<p><span>DNS</span><strong>";
  page += WiFi.dnsIP().toString();
  page += "</strong></p>";
  page += "<p><span>MAC</span><strong>";
  page += WiFi.macAddress();
  page += "</strong></p>";
  page += "</div></section>";
  page += "</main>";

  // Step 6: Add navigation buttons and close the HTML page.
  page += "<section class='actions'>";
  page += "<a class='button' href='/'>Dashboard</a>";
  page += "<a class='button secondary' href='/debug'>Refresh</a>";
  page += "</section>";
  appendPageEnd(page);

  // Step 7: Send the debug page to the browser.
  server.send(200, "text/html", page);
}

// Browsers often ask for /favicon.ico automatically. 204 means "no content".
void handleFavicon() {
  // Step 1: Tell the browser there is no favicon file.
  server.send(204);
}

// Handles any unknown URL.
void handleNotFound() {
  // Step 1: Log the unknown URL in the Serial Monitor.
  Serial.print("No handler for: ");
  Serial.println(server.uri());

  // Step 2: Send a friendly 404 page to the browser.
  sendMessagePage(404, "Not found", "That dashboard route does not exist.", "/", "Open dashboard");
}

// Escapes text before putting it into HTML, so unusual Wi-Fi names do not break the page.
String htmlEscape(const String& text) {
  // Step 1: Create a new String for the safe HTML version.
  String escaped;
  escaped.reserve(text.length() + 8);

  // Step 2: Check every character and replace special HTML characters.
  for (size_t i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (c == '&') {
      escaped += "&amp;";
    } else if (c == '<') {
      escaped += "&lt;";
    } else if (c == '>') {
      escaped += "&gt;";
    } else if (c == '"') {
      escaped += "&quot;";
    } else if (c == '\'') {
      escaped += "&#39;";
    } else {
      escaped += c;
    }
  }

  // Step 3: Return the escaped string.
  return escaped;
}

// Converts real RSSI signal strength in dBm into a simple 0-100 display percentage.
int wifiSignalPercent(int rssi) {
  // Step 1: Very weak signal maps to 0%.
  if (rssi <= -90) {
    return 0;
  }

  // Step 2: Very strong signal maps to 100%.
  if (rssi >= -40) {
    return 100;
  }

  // Step 3: Values between -90 and -40 are scaled linearly.
  return (rssi + 90) * 2;
}

// Turns the real RSSI value into a human-friendly label.
const char* wifiSignalLabel(int rssi) {
  // Step 1: -55 dBm or better is strong.
  if (rssi >= -55) {
    return "Strong";
  }

  // Step 2: -70 dBm or better is still good.
  if (rssi >= -70) {
    return "Good";
  }

  // Step 3: Anything lower is weak.
  return "Weak";
}

// Converts the ESP32 Wi-Fi mode enum into readable text for the debug page.
const char* wifiModeLabel(wifi_mode_t mode) {
  // Step 1: Check each known Wi-Fi mode and return readable text.
  if (mode == WIFI_OFF) {
    return "Off";
  }
  if (mode == WIFI_STA) {
    return "STA";
  }
  if (mode == WIFI_AP) {
    return "AP";
  }
  if (mode == WIFI_AP_STA) {
    return "AP+STA";
  }

  // Step 2: Fallback text for any unexpected mode value.
  return "Unknown";
}

// Adds the shared HTML header and CSS used by every page.
void appendPageStart(String& page, const char* title) {
  // Step 1: Add the document type, language, and start of the HTML head.
  page += "<!doctype html><html lang='en'><head>";

  // Step 2: Add browser metadata for text encoding and mobile scaling.
  page += "<meta charset='utf-8'>";
  page += "<meta name='viewport' content='width=device-width,initial-scale=1'>";

  // Step 3: Add the browser tab title.
  page += "<title>";
  page += title;
  page += "</title>";

  // Step 4: Add CSS styles. These styles are shared by the dashboard, setup, debug, and message pages.
  page += "<style>";
  page += ":root{color-scheme:light;--bg:#eef2f6;--panel:#ffffff;--ink:#17202a;--muted:#667485;--line:#d9e1ea;--blue:#2563eb;--green:#119c63;--amber:#b7791f;}";
  page += "*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font-family:Inter,Arial,sans-serif;line-height:1.45}";
  page += ".shell{width:min(980px,calc(100% - 28px));margin:0 auto;padding:28px 0 36px}";
  page += ".topbar{display:flex;align-items:flex-start;justify-content:space-between;gap:16px;margin-bottom:18px}";
  page += ".eyebrow{margin:0 0 4px;color:var(--muted);font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:0}";
  page += "h1{margin:0;font-size:clamp(28px,5vw,42px);line-height:1.05;letter-spacing:0}p{margin:0}";
  page += ".status-pill{display:inline-flex;align-items:center;border:1px solid var(--line);border-radius:999px;background:#fff;padding:7px 11px;font-size:13px;font-weight:700;white-space:nowrap}";
  page += ".status-pill.online{color:var(--green)}.status-pill.setup{color:var(--amber)}";
  page += ".summary{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px;margin-bottom:14px}";
  page += ".summary>div,.panel{background:var(--panel);border:1px solid var(--line);border-radius:8px;box-shadow:0 10px 26px rgba(28,39,54,.06)}";
  page += ".summary>div{padding:14px 16px}.label,.panel-title{display:block;color:var(--muted);font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:0}.summary strong{display:block;margin-top:3px;font-size:16px;overflow-wrap:anywhere}";
  page += ".grid{display:grid;grid-template-columns:1.3fr 1fr 1fr;gap:14px}.setup-grid{display:grid;grid-template-columns:1.15fr .85fr;gap:14px}";
  page += ".panel{padding:18px}.primary{min-height:220px}.metric-row{display:flex;justify-content:space-between;gap:16px;align-items:center;margin-top:14px}.metric{display:block;font-size:48px;font-weight:800;line-height:1}.metric-label{color:var(--muted);font-weight:700}";
  page += ".status-dot{width:14px;height:14px;border-radius:50%;background:var(--green);box-shadow:0 0 0 6px rgba(17,156,99,.13)}";
  page += ".meter{height:10px;background:#e6edf4;border-radius:999px;overflow:hidden;margin:20px 0}.meter span{display:block;height:100%;background:linear-gradient(90deg,var(--green),var(--blue));border-radius:999px}";
  page += ".detail-list{display:grid;gap:10px;margin-top:14px}.detail-list p{display:flex;justify-content:space-between;gap:18px;border-bottom:1px solid var(--line);padding-bottom:10px}.detail-list p:last-child{border-bottom:0;padding-bottom:0}.detail-list span{color:var(--muted)}.detail-list strong{text-align:right;overflow-wrap:anywhere}";
  page += ".actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:14px}.button,button{appearance:none;border:0;border-radius:8px;background:var(--blue);color:#fff;display:inline-flex;align-items:center;justify-content:center;min-height:42px;padding:0 16px;font-weight:800;text-decoration:none;cursor:pointer}.button.secondary{background:#fff;color:var(--ink);border:1px solid var(--line)}";
  page += "form{display:grid;gap:10px;margin-top:14px}label{font-weight:800}input{width:100%;min-height:44px;border:1px solid var(--line);border-radius:8px;padding:0 12px;font-size:16px;background:#fff;color:var(--ink)}";
  page += ".message{max-width:640px}.message p{color:var(--muted);margin:12px 0 18px}";
  page += "@media(max-width:760px){.shell{width:min(100% - 20px,980px);padding-top:18px}.topbar,.summary,.grid,.setup-grid{grid-template-columns:1fr}.topbar{display:grid}.metric{font-size:40px}.detail-list p{display:grid;gap:2px}.detail-list strong{text-align:left}}";

  // Step 5: Close the head and start the visible page wrapper.
  page += "</style></head><body><div class='shell'>";
}

// Closes the shared HTML wrapper.
void appendPageEnd(String& page) {
  // Step 1: Close the wrapper, body, and html tags.
  page += "</div></body></html>";
}

// Sends a simple styled message page for errors and success states.
void sendMessagePage(int code, const char* title, const char* message, const char* linkHref, const char* linkText) {
  // Step 1: Create a String to hold the message page.
  String page;
  page.reserve(2200);

  // Step 2: Add the shared page header and CSS.
  appendPageStart(page, title);

  // Step 3: Add the message content and one button.
  page += "<section class='panel message'>";
  page += "<span class='panel-title'>Cube OS</span>";
  page += "<h1>";
  page += title;
  page += "</h1><p>";
  page += message;
  page += "</p>";
  page += "<a class='button' href='";
  page += linkHref;
  page += "'>";
  page += linkText;
  page += "</a></section>";

  // Step 4: Close the HTML page.
  appendPageEnd(page);

  // Step 5: Send the message page with the requested HTTP status code.
  server.send(code, "text/html", page);
}
