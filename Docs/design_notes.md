# Design Notes

## Goals

- Build a physical friend-presence robot that can communicate with other Cube OS units through online servers.
- Make the robot expressive enough for voice notes, messages, remote control, camera sharing, and display-based reactions.
- Keep the robot small enough for a desk while still supporting two-legged movement, battery power, USB-C charging, four displays, audio, sensors, and a camera.
- Preserve a reviewer-friendly repo with current KiCad files, CAD exports, production files, sourcing notes, firmware, and testing records.

## Decisions

| Decision | Reason | Tradeoff |
| --- | --- | --- |
| ESP32-S3-WROOM-1-N8R8 as main MCU | Provides Wi-Fi, USB, camera interface potential, enough GPIO for displays/sensors/audio, and PSRAM support for media features | Firmware must carefully manage pins, memory, camera/display bandwidth, and sleep states |
| Four separate face displays | Gives the robot a cube-like expressive interface for widgets, friend feeds, and status | Adds FPC routing, display synchronization, backlight power, and enclosure complexity |
| Two-servo linkage walking | Keeps locomotion mechanically interesting while limiting motor count and cost | Gait control is constrained by linkage geometry and servo torque |
| Online server plus dashboard | Enables long-distance friend interaction, pairing, remote control, clips, and telemetry | Requires authentication, rate limiting, quiet hours, uptime, and privacy choices |
| Ambient-light rest mode | Lets the robot back off when the room is dark or the user wants quiet | Needs careful thresholds so normal lighting changes do not trigger annoying mode switches |

## Open Questions

- Should OV2640 reset be connected to a GPIO or handled with a fixed reset circuit in the next PCB revision?
- Should charger status and battery percentage be available in firmware, and if so, should Rev B add a battery fuel gauge?
- Which display controller and graphics library will be used for the ER-TFT1.54-2 panels after datasheet validation?
- What is the minimum safe servo power budget during walking, turning, stall, and startup?
- Which features require privacy controls: camera, microphone, remote driving, and 2 a.m. notifications?
- What are the final mechanical limits for Linkage V2: stride length, foot clearance, center of mass, and linkage hard stops?
