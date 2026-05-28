#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "Dashboard.h"
#include "Network.h"
#include "config.h"

namespace {

// The web server listens on the configured HTTP port.
WebServer server(WEB_SERVER_PORT);

bool webServerStarted = false;
bool setupModeActive = false;

}

void setSetupModeActive(bool active) {
  setupModeActive = active;
}

bool isSetupModeActive() {
  return setupModeActive;
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

void handleWebServerClient() {
  if (webServerStarted) {
    server.handleClient();
  }
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
  page += MDNS_NAME;
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
  page += MDNS_NAME;
  page += ".local</strong></p>";
  page += "<p><span>mDNS status</span><strong>";
  page += isMdnsStarted() ? "Started" : "Not started";
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
  page += MDNS_NAME;
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
  page += AP_NAME;
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
  page += isMdnsStarted() ? "<span class='status-pill online'>mDNS started</span>" : "<span class='status-pill setup'>mDNS not started</span>";
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
  page += MDNS_NAME;
  page += "</strong></p>";
  page += "<p><span>mDNS URL</span><strong>http://";
  page += MDNS_NAME;
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
