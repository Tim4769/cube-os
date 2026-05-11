# Testing Plan

## Electrical

- Check for shorts between USB_VBUS, +BAT, 3.3 V, camera AVDD/DVDD rails, 1.2 V, 2.5 V, and GND before power-up.
- Power from current-limited USB first and verify USB_VBUS at the charger input.
- Verify BQ24074 charger/power-path behavior with and without the 1S LiPo connected.
- Verify TPS63802 output and downstream 2.5 V / 1.2 V camera rails before plugging in the OV2640.
- Verify servo rail voltage under idle, sweep, and stall-risk conditions before walking tests.
- Confirm the USB-C port supports flashing and serial logs.

## Firmware

- Confirm serial output from the ESP32-S3 over USB.
- Add a Rev A pin-map test before feature firmware.
- Run I2C scan for SHT45, VEML7700, and ISM330DHCX.
- Run display tests for top, front, right, and left FPC connectors.
- Run audio tests: microphone level meter and speaker tone/playback.
- Run camera tests: I2C probe, XCLK output, frame capture, JPEG upload or serial save.
- Run servo tests: individual sweep, paired sweep, gait timing, and emergency stop.
- Run network tests: Wi-Fi connect, server reconnect, robot pairing, message delivery, and quiet-hour enforcement.

## Mechanical

- Confirm PCB fit against the final enclosure and Linkage V2 body.
- Confirm USB-C, power switch, boot/reset access, battery connector, speaker connector, servo headers, camera connector, and display FPC access.
- Confirm buttons, FPCs, battery, speaker, and wires clear the moving linkage.
- Confirm foot clearance, stride length, center of mass, and turning behavior on a desk surface.
- Confirm mechanical hard stops prevent servo overtravel and linkage lockup.
- Confirm sensor openings for microphone, speaker, camera, light, and temperature/humidity are not blocked.

## Final Acceptance

- PCB DRC has 0 violations and schematic ERC issues are either fixed or documented.
- All regulated rails are verified under expected load.
- Each display, sensor, servo, microphone, speaker, camera, USB, and Wi-Fi feature passes its bring-up test.
- Robot can walk forward, turn left, turn right, enter rest mode, and recover safely from a fall or stuck state.
- Dashboard can pair two robots, send a message, send/play a voice note, trigger a safe remote-control command, and show telemetry.
- Mechanical assembly can be opened for service without damaging FPCs, cables, or linkage pivots.

## Current KiCad Check Record

- May 11, 2026 PCB DRC: 0 violations and 0 unconnected items.
- May 11, 2026 schematic ERC: 1 error and 37 warnings.
- ERC error to fix: OV2640 symbol has duplicate pin numbers.
