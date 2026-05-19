# Rev A Pin Map Guide

This guide is based on the KiCad Rev A schematic at
`PCB/Rev A/source/Cube OS/Cube OS.kicad_sch`.

Use this as the source for filling in `Firmware/src/config.h`. The important
part of every connection is:

- `U1` is the ESP32-S3-WROOM-1-N8R8.
- The schematic net name tells you the signal purpose.
- The `U1` pin function tells you the ESP32 GPIO to use in firmware.
- The other nodes on the same net tell you what that GPIO is connected to.

## How to Find a Connection

In KiCad:

1. Open `PCB/Rev A/source/Cube OS/Cube OS.kicad_sch`.
2. Search for the signal label, for example `SERVO1_PWM` or `DISP_MOSI`.
3. Highlight the net.
4. Find the `U1` pin on that highlighted net.
5. Use the `IOxx` number from `U1` as the firmware GPIO number.

From the terminal:

```bash
kicad-cli sch export netlist --format kicadxml \
  -o /private/tmp/cubeos_rev_a.xml \
  "PCB/Rev A/source/Cube OS/Cube OS.kicad_sch"
```

Then search the exported netlist for the signal:

```bash
rg -n 'name="/SERVO1_PWM"|ref="U1"' /private/tmp/cubeos_rev_a.xml
```

## Config Constants

This is the practical block to mirror into `Firmware/src/config.h`.

```cpp
constexpr int PIN_UNASSIGNED = -1;

// Rev A has no separate status LED GPIO in the schematic.
// Do not use GPIO48 for status LED: GPIO48 is CAM_PCLK.
constexpr int STATUS_LED_PIN = PIN_UNASSIGNED;

constexpr int I2C_SDA_PIN = 8;
constexpr int I2C_SCL_PIN = 9;

constexpr int DISPLAY_TOP_CS_PIN = 4;
constexpr int DISPLAY_FRONT_CS_PIN = 6;
constexpr int DISPLAY_LEFT_CS_PIN = 7;
constexpr int DISPLAY_RIGHT_CS_PIN = 15;
constexpr int DISPLAY_DC_PIN = 10;
constexpr int DISPLAY_MOSI_PIN = 11;
constexpr int DISPLAY_SCLK_PIN = 12;
constexpr int DISPLAY_BACKLIGHT_PWM_PIN = 13;
constexpr int DISPLAY_RESET_PIN = 14;

constexpr int I2S_BCLK_PIN = 16;
constexpr int I2S_LRCLK_PIN = 21;
constexpr int I2S_MIC_DATA_PIN = 40;
constexpr int I2S_SPK_DATA_PIN = 42;

constexpr int IMU_INT1_PIN = 17;
constexpr int IMU_INT2_PIN = 18;

constexpr int MOVEMENT_SERVO1_PWM_PIN = 1; // M1
constexpr int MOVEMENT_SERVO2_PWM_PIN = 2; // M2

// Only assign these after confirming which physical servo is left/right.
constexpr int MOVEMENT_LEFT_SERVO_PIN = PIN_UNASSIGNED;
constexpr int MOVEMENT_RIGHT_SERVO_PIN = PIN_UNASSIGNED;

constexpr int USER_SWITCH_PIN = 5;

constexpr int CHARGER_PGOOD_PIN = 38;
constexpr int CHARGER_STAT_PIN = 39;

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

constexpr int USB_D_N_PIN = 19;
constexpr int USB_D_P_PIN = 20;
```

## ESP32 GPIO to Device Map

| Signal | ESP32 GPIO / function | Device connection |
| --- | --- | --- |
| `SERVO1_PWM` | GPIO1 | `M1` servo PWM |
| `SERVO2_PWM` | GPIO2 | `M2` servo PWM |
| `CAM_HREF` | GPIO3 | `CAM1` OV2640 HREF |
| `DISP_TOP_CS` | GPIO4 | `U7` top display CS |
| `USER_SWITCH` | GPIO5 | `SW2` through `R16` |
| `DISP_FRONT_CS` | GPIO6 | `U9` front display CS |
| `DISP_LEFT_CS` | GPIO7 | `U12` left display CS |
| `I2C_SDA` | GPIO8 | OV2640 SCCB SDA, SHT45 SDA, VEML7700 SDA, ISM330DHCX SDA |
| `I2C_SCL` | GPIO9 | OV2640 SCCB SCL, SHT45 SCL, VEML7700 SCL, ISM330DHCX SCL |
| `DISP_DC` | GPIO10 | Shared display D/C |
| `DISP_MOSI` | GPIO11 | Shared display SDA/MOSI |
| `DISP_SCLK` | GPIO12 | Shared display SCLK |
| `DISP_BL_PWM` | GPIO13 | `Q1` display backlight MOSFET gate |
| `DISP_RST` | GPIO14 | Shared display reset |
| `DISP_RIGHT_CS` | GPIO15 | `U10` right display CS |
| `I2S_BCLK` | GPIO16 | `MK1` mic SCK and `U13` amp BCLK |
| `IMU_INT1` | GPIO17 | `U5` ISM330DHCX INT1 |
| `IMU_INT2` | GPIO18 | `U5` ISM330DHCX INT2 |
| `USB_D_N` | GPIO19 | USB-C D- |
| `USB_D_P` | GPIO20 | USB-C D+ |
| `I2S_LRCLK` | GPIO21 | `MK1` mic WS and `U13` amp LRCLK |
| `CAM_D0` | GPIO35 | OV2640 DATA2 |
| `CAM_D1` | GPIO36 | OV2640 DATA3 |
| `CAM_D2` | GPIO37 | OV2640 DATA4 |
| `CHG_PGOOD` | GPIO38 | `U8` BQ24074 `*PGOOD` through `R13` |
| `CHG_STAT` | GPIO39 | `U8` BQ24074 `*CHG` through `R12` |
| `I2S_MIC_DATA` | GPIO40 | `MK1` mic SD |
| `CAM_XCLK` | GPIO41 | OV2640 XCLK |
| `I2S_SPK_DATA` | GPIO42 | `U13` MAX98357A DIN |
| `CAM_D3` | GPIO43 / TXD0 | OV2640 DATA5 |
| `CAM_D4` | GPIO44 / RXD0 | OV2640 DATA6 |
| `CAM_D5` | GPIO45 | OV2640 DATA7 |
| `CAM_D6` | GPIO46 | OV2640 DATA8 |
| `CAM_D7` | GPIO47 | OV2640 DATA9 |
| `CAM_PCLK` | GPIO48 | OV2640 PCLK |

## Shared Buses

I2C/SCCB bus:

| Signal | GPIO | Connected parts |
| --- | --- | --- |
| `I2C_SDA` | GPIO8 | OV2640 `SIO_D`, SHT45 `SDA`, VEML7700 `SDA`, ISM330DHCX `SDA`, pullup `R2` |
| `I2C_SCL` | GPIO9 | OV2640 `SIO_C`, SHT45 `SCL`, VEML7700 `SCL`, ISM330DHCX `SCL`, pullup `R1` |

Display SPI-like bus:

| Signal | GPIO | Connected parts |
| --- | --- | --- |
| `DISP_MOSI` | GPIO11 | All four display `SDA` pins |
| `DISP_SCLK` | GPIO12 | All four display `SCLK` pins |
| `DISP_DC` | GPIO10 | All four display `D/C` pins |
| `DISP_RST` | GPIO14 | All four display `RESET` pins |
| `DISP_BL_PWM` | GPIO13 | Backlight MOSFET `Q1` gate |
| `DISP_TOP_CS` | GPIO4 | Top display `U7` |
| `DISP_FRONT_CS` | GPIO6 | Front display `U9` |
| `DISP_RIGHT_CS` | GPIO15 | Right display `U10` |
| `DISP_LEFT_CS` | GPIO7 | Left display `U12` |

I2S audio:

| Signal | GPIO | Connected parts |
| --- | --- | --- |
| `I2S_BCLK` | GPIO16 | Mic `MK1` SCK, amp `U13` BCLK |
| `I2S_LRCLK` | GPIO21 | Mic `MK1` WS, amp `U13` LRCLK |
| `I2S_MIC_DATA` | GPIO40 | Mic `MK1` SD |
| `I2S_SPK_DATA` | GPIO42 | Amp `U13` DIN |

## Notes and Gotchas

- `STATUS_LED_PIN 48` in the current starter firmware is not valid for Rev A.
  GPIO48 is wired to OV2640 `PCLK`.
- The camera reset pin is not on an ESP32 GPIO in Rev A. It is pulled up to
  3.3 V by `R14`.
- The camera power-down pin is tied to GND, so it is always enabled.
- `CAM_D3` and `CAM_D4` use the ESP32-S3 UART0 pins. In Arduino-style GPIO
  numbering, use GPIO43 for TXD0 and GPIO44 for RXD0.
- `CHG_PGOOD` and `CHG_STAT` reach GPIO38/GPIO39 through `R13`/`R12`.
  Verify the charger status input behavior during board bring-up before
  relying on these signals for product logic.
- `M1` and `M2` are schematic names only. Confirm which physical servo is left
  and right during assembly before assigning `MOVEMENT_LEFT_SERVO_PIN` and
  `MOVEMENT_RIGHT_SERVO_PIN`.
- Native USB uses GPIO19/GPIO20. Do not reuse them for application GPIO.
- `Firmware/platformio.ini` currently uses `board = esp32dev`, while Rev A is
  ESP32-S3-WROOM-1-N8R8. Update the PlatformIO board target before relying on
  ESP32-S3-specific pins, native USB, camera, or PSRAM behavior.
