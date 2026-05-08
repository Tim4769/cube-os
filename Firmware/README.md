# Firmware

Starter firmware for Cube OS.

## Toolchain

This scaffold is set up for PlatformIO with an ESP32-S3 Arduino target.

## Build

```bash
pio run
```

## Upload

```bash
pio run --target upload
```

## Serial Monitor

```bash
pio device monitor
```

Update `platformio.ini`, `src/config.h`, and `src/main.cpp` once the final MCU, pins, and board package are chosen.
