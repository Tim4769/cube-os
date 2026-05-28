#include <Arduino.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

#include "Network.h"
#include "config.h"

namespace {

// Preferences is Arduino's wrapper around ESP32 NVS flash storage.
Preferences preferences;

bool mdnsStarted = false;
unsigned long lastMdnsAttemptAt = 0;

}

// Reads saved Wi-Fi credentials from NVS flash storage.
// The & symbols mean this function fills in the caller's ssid and password variables.
bool loadWiFiCredentials(String& ssid, String& password) {
  // Step 1: Open the "wifi" namespace in read-only mode.
  if (!preferences.begin(WIFI_NAMESPACE, true)) {
    Serial.println("Failed to open NVS for reading.");
    return false;
  }

  // Step 2: Read the configured flag to see if Wi-Fi was saved before.
  bool configured = preferences.getBool(WIFI_CONFIGURED_KEY, false);

  // Step 3: If configured is false, there are no saved credentials to use.
  if (!configured) {
    preferences.end();
    return false;
  }

  // Step 4: Read the saved SSID and password from NVS.
  ssid = preferences.getString(WIFI_SSID_KEY, "");
  password = preferences.getString(WIFI_PASSWORD_KEY, "");

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
  WiFi.setHostname(MDNS_NAME);

  // Step 4: Wait briefly so the Wi-Fi mode change can settle.
  delay(100);

  // Step 5: Start connecting to the requested Wi-Fi network.
  WiFi.begin(ssid.c_str(), password.c_str());

  // Step 6: Wait for Wi-Fi, but give up after WIFI_CONNECT_TIMEOUT_MS so the ESP32 does not hang forever.
  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(WIFI_CONNECT_CHECK_INTERVAL_MS);
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
  bool ok = WiFi.softAP(AP_NAME, AP_PASSWORD);

  // Step 3: Stop here if the setup network could not start.
  if (!ok) {
    Serial.println("Failed to start setup AP");
    return false;
  }

  // Step 4: Print the setup network information for the Serial Monitor.
  Serial.println("Setup access point started");
  Serial.print("Network name: ");
  Serial.println(AP_NAME);
  Serial.print("Setup URL: http://");
  Serial.println(WiFi.softAPIP());

  // Step 5: Return true so startSetupMode() knows the AP is ready.
  return true;
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
  WiFi.setHostname(MDNS_NAME);

  // Step 4: Start the mDNS responder. This creates cubeos.local.
  if (!MDNS.begin(MDNS_NAME)) {
    Serial.println("Failed to start mDNS. Will retry.");
    mdnsStarted = false;
    return false;
  }

  // Step 5: Advertise that this device has an HTTP web server on port 80.
  MDNS.addService("http", "tcp", WEB_SERVER_PORT);
  MDNS.addServiceTxt("http", "tcp", "path", "/");

  // Step 6: Remember that mDNS is running and print the local URL.
  mdnsStarted = true;

  Serial.print("mDNS address: http://");
  Serial.print(MDNS_NAME);
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

  // Step 3: If mDNS is not running, retry every MDNS_RETRY_INTERVAL_MS.
  if (!mdnsStarted && millis() - lastMdnsAttemptAt >= MDNS_RETRY_INTERVAL_MS) {
    startMdns();
  }
}

// Saves Wi-Fi credentials to NVS only after a successful connection test.
bool saveWiFiCredentials(const String& ssid, const String& password) {
  // Step 1: Open the "wifi" namespace in read/write mode.
  if (!preferences.begin(WIFI_NAMESPACE, false)) {
    Serial.println("Failed to open NVS for writing.");
    return false;
  }

  // Step 2: Save the SSID, password, and configured flag.
  preferences.putString(WIFI_SSID_KEY, ssid);
  preferences.putString(WIFI_PASSWORD_KEY, password);
  preferences.putBool(WIFI_CONFIGURED_KEY, true);

  // Step 3: Close NVS after writing.
  preferences.end();

  // Step 4: Return true so the caller knows the save worked.
  Serial.println("Wi-Fi credentials saved to NVS.");
  return true;
}

// Not called by the normal dashboard. This is here for a future factory-reset button.
void clearWiFiCredentials() {
  // Step 1: Open the "wifi" namespace in read/write mode.
  if (!preferences.begin(WIFI_NAMESPACE, false)) {
    Serial.println("Failed to open NVS for writing.");
    return;
  }

  // Step 2: Remove each Wi-Fi-related key.
  preferences.remove(WIFI_SSID_KEY);
  preferences.remove(WIFI_PASSWORD_KEY);
  preferences.remove(WIFI_CONFIGURED_KEY);

  // Step 3: Close NVS after removing the data.
  preferences.end();

  Serial.println("Wi-Fi credentials cleared.");
}

bool isMdnsStarted() {
  return mdnsStarted;
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
