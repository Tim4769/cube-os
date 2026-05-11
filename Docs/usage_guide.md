# Usage Guide

## Power On

- Charge the 1S LiPo through USB-C before untethered use.
- Power the first assembled board from a current-limited supply during bring-up.
- Do not run walking tests until the servo rail and linkage clearances have been verified.

## Controls

- Physical controls include the board switches plus USB-C for flashing, charging, and serial debugging.
- The four displays are planned for face widgets, friend status, messages, camera previews, time, and environmental data.
- The dashboard is planned for pairing, remote control, voice notes, text messages, quiet hours, camera clips, and telemetry.
- Friend remote control should always respect quiet hours, safety limits, and a local physical override.

## Normal Operation

- Cube OS connects to Wi-Fi, signs into the server, and appears online to paired friends.
- Friends can send messages, voice notes, sounds, reactions, and safe movement commands.
- The robot can walk forward, turn left, turn right, enter rest mode, and show widgets across the four displays.
- The robot can monitor local temperature, humidity, ambient light, motion state, and eventually battery state.

## Shutdown

- Stop active movement before powering down.
- Use the physical power control for storage or transport.
- Store with the linkage relaxed, displays protected, and battery at a safe storage charge when possible.
- Disconnect USB-C and avoid leaving the robot in a position where the legs or FPC cables are under load.
