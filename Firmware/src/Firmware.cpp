#include <Arduino.h>
#include <WiFi.h>

#include "Dashboard.h"
#include "Movement.h"
#include "Network.h"
#include "config.h"

namespace {

void startSetupMode();
void startNormalMode();

}

void firmwareSetup() {
  // Step 1: Start the Serial Monitor so we can see debug messages on the computer.
  Serial.begin(SERIAL_BAUD_RATE);

  // Step 2: Wait a little so the ESP32 and Serial Monitor have time to settle after boot.
  delay(BOOT_STABILIZE_DELAY_MS);

  // Step 3: Print why the ESP32 restarted. This is useful when debugging.
  Serial.print("Reset reason: ");
  Serial.println(esp_reset_reason());

  // Step 4: Prepare movement hardware hooks. This is a no-op until movement pins are assigned.
  setupMovement();

  // Step 5: Create empty variables where saved Wi-Fi credentials can be loaded.
  String savedSsid;
  String savedPassword;

  // Step 6: Try to load Wi-Fi details that were saved earlier in NVS.
  if (loadWiFiCredentials(savedSsid, savedPassword)) {
    Serial.println("Trying saved Wi-Fi credentials...");

    // Step 7: If saved Wi-Fi works, go straight to normal dashboard mode.
    if (connectToWiFi(savedSsid, savedPassword)) {
      startNormalMode();
      return;
    }

    // Step 8: If saved Wi-Fi fails, do not erase it. Start setup mode instead.
    Serial.println("Saved Wi-Fi failed. Starting setup mode without deleting saved credentials.");
  } else {
    // Step 9: If there are no saved credentials at all, start setup mode.
    Serial.println("No saved Wi-Fi credentials. Starting setup mode.");
  }

  // Step 10: Setup mode lets a user enter Wi-Fi details through a browser.
  startSetupMode();
}

void firmwareLoop() {
  // Step 1: Keep mDNS alive so cubeos.local can keep working.
  maintainMdns();

  // Step 2: Let movement update without blocking the web server.
  updateMovement();

  // Step 3: If the web server is running, check for browser requests.
  handleWebServerClient();
}

namespace {

// Setup mode starts a temporary ESP32 Wi-Fi network and shows the Wi-Fi setup form.
void startSetupMode() {
  // Step 1: Remember that the ESP32 is currently in setup mode.
  setSetupModeActive(true);

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
  setSetupModeActive(false);

  // Step 2: Print the IP address. You can open this IP in a browser.
  Serial.println("Normal mode started");
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());

  // Step 3: Start the dashboard web server.
  startWebServerOnce();

  // Step 4: Start mDNS so http://cubeos.local can work on the same Wi-Fi.
  startMdns();
}

}
