#include <Arduino.h>

#include "Firmware.h"

// setup() runs once every time the ESP32 boots or restarts.
void setup() {
  firmwareSetup();
}

// loop() runs forever after setup(). It must stay quick so the web server can respond.
void loop() {
  firmwareLoop();
}
