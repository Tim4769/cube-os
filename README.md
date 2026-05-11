# Cube OS

Two-legged desktop telepresence robot for keeping long-distance friends connected through physical, playful interaction.

## Project

- Repo: [Tim4769/cube-os](https://github.com/Tim4769/cube-os)
- Team: Tim, Pat, Taiyo
- Concept: Wi-Fi friend robot with walking, voice notes, camera clips, dashboard control, environmental sensing, and four expressive display faces.

## Why It Exists

After 7 years at the same school, some team members are graduating and moving away. Cube OS turns remote friendship into a physical desk companion: messages, sounds, robot nudges, camera moments, and shared presence beyond group chats.

## Top Features

- Telepresence Network: robot-to-robot Wi-Fi messaging through an online server.
- Bipedal Locomotion: two 9 g servos driving a four-bar walking linkage.
- Inertial Gait Control: IMU-assisted stepping, turning, fall detection, and recovery.
- Ambient Rest Mode: VEML7700 light-aware quiet/do-not-disturb behavior.
- Desk Climate Sensing: SHT45 temperature and humidity monitoring.
- Voice Mailbox: ICS-43434 mic recording and MAX98357A speaker playback.
- USB-C Workflow: charging, flashing, serial logs, and battery-powered use.
- Camera Sharing: OV2640 snapshots or short clips to dashboard/friends.
- Four-Face Display OS: widgets, expressions, friend feeds, time, status, and air data.
- Social Dashboard: pairing, chat, voice, remote control, camera preview, and telemetry.

## Hardware

- MCU: ESP32-S3-WROOM-1-N8R8
- Sensors: ISM330DHCX IMU, SHT45, VEML7700, ICS-43434, 10 kOhm NTC
- Camera: OV2640 connector
- Displays: four 1.54 in display FPC connectors
- Motion: two 3-pin servo headers
- Audio: MAX98357A I2S amplifier and speaker connector
- Power: USB-C, 1S LiPo, BQ24074 charger, TPS63802 buck-boost, 2.5 V/1.2 V camera rails
- PCB: Rev A, KiCad 9.0, 60.05 mm x 60.05 mm, 4-layer FR4

## Active Files

- PCB source: `PCB/Rev A/source/Cube OS/`
- PCB production: `PCB/Rev A/production/`
- Gerbers: `PCB/Rev A/gerbers/Gerber.zip`
- CAD linkage: `CAD/Legs/Linkage V1/`, `CAD/Legs/Linkage V2/`
- Firmware: `Firmware/`
- Dashboard placeholder: `Software/`
- Feature roadmap: `Docs/feature_roadmap.md`

## Empty Placeholders

These folders are intentionally present but do not contain final project files yet:

- `Media/renders/`: no final renders yet
- `Media/photos/`: no build photos yet
- `Media/diagrams/`: no diagrams yet
- `PCB/Rev A/3d_models/`: no PCB 3D render exports yet
- `Software/src/`: no dashboard source yet
- `Software/screenshots/`: no dashboard screenshots yet
- `Zine/`: no final zine export yet
- `Zine/source/`: no editable zine source yet

`PCB/Template/` is reference data, not active Rev A release data.

## BOM Snapshot

| Item | Qty | Estimate |
| --- | ---: | ---: |
| PCB minimum production order | 2 | 144 USD |
| 9 g micro servo motors | 2 | 8 USD |
| OV2640 camera | 1 | 5 USD |
| AS02504AR speaker | 1 | 5 USD |
| 1S 3.7 V LiPo battery | 1 | 6 USD |
| ER-TFT1.54-2 displays | 4 | 24 USD |
| **Known subtotal** |  | **192 USD** |

Detailed sourcing: `BOM/bom.csv` and `BOM/sourcing_notes.md`.

## Build

```bash
cd Firmware
pio run
pio run --target upload
```

## Checks

- PCB DRC: 0 violations, 0 unconnected items.
- Schematic ERC: 1 error, 37 warnings.
- Open electrical cleanup: OV2640 duplicate symbol pin numbers and library pin-type warnings.
- Bring-up plan: `Docs/testing_plan.md`

## Past Projects

- [Microver](https://github.com/Tim4769/Microver): compact rover for the [Swiss Rover Challenge](https://swissroverchallenge.com/).
- [VEX Robotics 15520X](https://github.com/rosey-15520X): autonomous navigation and game-element manipulation.

## License

MIT. See `LICENSE`.
