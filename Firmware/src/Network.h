#pragma once

#include <Arduino.h>
#include <WiFi.h>

bool loadWiFiCredentials(String& ssid, String& password);
bool connectToWiFi(const String& ssid, const String& password);
bool startSetupAccessPoint();
bool startMdns();
void maintainMdns();
bool saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();
bool isMdnsStarted();
int wifiSignalPercent(int rssi);
const char* wifiSignalLabel(int rssi);
const char* wifiModeLabel(wifi_mode_t mode);
