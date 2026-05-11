# Firmware

PlatformIO starter firmware for Cube OS on ESP32-S3 Arduino.

## Commands

```bash
pio run
pio run --target upload
pio device monitor
```

## Next Firmware Targets

- Rev A pin map from KiCad nets
- I2C scan: SHT45, VEML7700, ISM330DHCX
- Display test: four SPI faces
- Audio test: ICS-43434 mic and MAX98357A speaker
- Camera test: OV2640 capture
- Motion test: dual servo gait sweep
- Network test: Wi-Fi, server pairing, telemetry

Update `platformio.ini`, `src/config.h`, and `src/main.cpp` as Rev A bring-up replaces the starter code.
