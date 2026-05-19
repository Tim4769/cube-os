#pragma once

#include <Arduino.h>

void setSetupModeActive(bool active);
bool isSetupModeActive();
void startWebServerOnce();
void handleWebServerClient();
void handleRoot();
void handleSetupPage();
void handleConnect();
void handleDebug();
void handleFavicon();
void handleNotFound();
String htmlEscape(const String& text);
void appendPageStart(String& page, const char* title);
void appendPageEnd(String& page);
void sendMessagePage(int code, const char* title, const char* message, const char* linkHref, const char* linkText);
