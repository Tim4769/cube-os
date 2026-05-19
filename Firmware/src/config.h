#pragma once

#define SERIAL_BAUD_RATE 115200

// Setup access point details. These are only used when the ESP32 needs Wi-Fi setup.
constexpr const char* AP_NAME = "CubeOS-Setup";
constexpr const char* AP_PASSWORD = "12345678";

// mDNS lets a browser open http://cubeos.local instead of typing the numeric IP address.
constexpr const char* MDNS_NAME = "cubeos";

// Names used inside ESP32 NVS flash storage for the saved Wi-Fi credentials.
constexpr const char* WIFI_NAMESPACE = "wifi";
constexpr const char* WIFI_SSID_KEY = "ssid";
constexpr const char* WIFI_PASSWORD_KEY = "password";
constexpr const char* WIFI_CONFIGURED_KEY = "configured";

// Timing values are in milliseconds.
constexpr unsigned long BOOT_STABILIZE_DELAY_MS = 3000;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr unsigned long WIFI_CONNECT_CHECK_INTERVAL_MS = 500;
constexpr unsigned long MDNS_RETRY_INTERVAL_MS = 5000;

// The web server listens on port 80, the normal HTTP port.
constexpr int WEB_SERVER_PORT = 80;

constexpr int PIN_UNASSIGNED = -1;

// Rev A has no separate status LED GPIO in the schematic.
// Do not use GPIO48 for status LED: GPIO48 is CAM_PCLK.
constexpr int STATUS_LED_PIN = PIN_UNASSIGNED;

// Shared I2C/SCCB bus.
constexpr int I2C_SDA_PIN = 8;
constexpr int I2C_SCL_PIN = 9;

// Four display connectors share MOSI/SCLK/DC/reset/backlight and use separate CS pins.
constexpr int DISPLAY_TOP_CS_PIN = 4;
constexpr int DISPLAY_FRONT_CS_PIN = 6;
constexpr int DISPLAY_LEFT_CS_PIN = 7;
constexpr int DISPLAY_RIGHT_CS_PIN = 15;
constexpr int DISPLAY_DC_PIN = 10;
constexpr int DISPLAY_MOSI_PIN = 11;
constexpr int DISPLAY_SCLK_PIN = 12;
constexpr int DISPLAY_BACKLIGHT_PWM_PIN = 13;
constexpr int DISPLAY_RESET_PIN = 14;

// I2S audio: ICS-43434 microphone and MAX98357A speaker amplifier.
constexpr int I2S_BCLK_PIN = 16;
constexpr int I2S_LRCLK_PIN = 21;
constexpr int I2S_MIC_DATA_PIN = 40;
constexpr int I2S_SPK_DATA_PIN = 42;

// ISM330DHCX interrupt lines.
constexpr int IMU_INT1_PIN = 17;
constexpr int IMU_INT2_PIN = 18;

// Servo connector PWM pins.
constexpr int MOVEMENT_SERVO1_PWM_PIN = 1; // M1
constexpr int MOVEMENT_SERVO2_PWM_PIN = 2; // M2

// Only assign these after confirming which physical servo is left/right.
constexpr int MOVEMENT_LEFT_SERVO_PIN = PIN_UNASSIGNED;
constexpr int MOVEMENT_RIGHT_SERVO_PIN = PIN_UNASSIGNED;

// User/control and charger status pins.
constexpr int USER_SWITCH_PIN = 5;
constexpr int CHARGER_PGOOD_PIN = 38;
constexpr int CHARGER_STAT_PIN = 39;

// OV2640 camera parallel bus and clock/sync pins.
constexpr int CAMERA_D0_PIN = 35;
constexpr int CAMERA_D1_PIN = 36;
constexpr int CAMERA_D2_PIN = 37;
constexpr int CAMERA_D3_PIN = 43; // schematic pin function TXD0
constexpr int CAMERA_D4_PIN = 44; // schematic pin function RXD0
constexpr int CAMERA_D5_PIN = 45;
constexpr int CAMERA_D6_PIN = 46;
constexpr int CAMERA_D7_PIN = 47;
constexpr int CAMERA_HREF_PIN = 3;
constexpr int CAMERA_PCLK_PIN = 48;
constexpr int CAMERA_VSYNC_PIN = 0;
constexpr int CAMERA_XCLK_PIN = 41;
constexpr int CAMERA_RESET_PIN = PIN_UNASSIGNED; // pulled up by R14
constexpr int CAMERA_PWDN_PIN = PIN_UNASSIGNED;  // tied to GND

// Native USB pins. Keep these reserved for USB.
constexpr int USB_D_N_PIN = 19;
constexpr int USB_D_P_PIN = 20;
