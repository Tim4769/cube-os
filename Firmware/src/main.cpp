#include <Arduino.h>
#include "config.h"

static bool led_state = false;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(STATUS_LED_PIN, OUTPUT);
  Serial.println("Cube OS firmware scaffold started");
}

void loop() {
  led_state = !led_state;
  digitalWrite(STATUS_LED_PIN, led_state ? HIGH : LOW);
  Serial.println("heartbeat");
  delay(1000);
}
