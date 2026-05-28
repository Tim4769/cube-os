# Cube OS

## Members

- Tim
- Pat
- Taiyo

## Description

Cube OS is a desktop robot of 80 x 80 x 80 mm controlled by two legs. It can walk and communicate with other Cube OS robots through online servers using a custom PCB with an ESP32 microcontroller. It supports voice messages, text messages, and remote control features, helping friends stay connected even when they are far apart.

## Features

- Wi-Fi communication between robots via an online server.
- Two 9 g servos power a four-bar walking linkage for bipedal locomotion.
- IMU-assisted stepping, turning, fall detection, and recovery for inertial gait control.
- Ambient Rest Mode with light-aware quiet, privacy, and do-not-disturb behavior.
- Desk climate sensing for local humidity and temperature data.
- Voice Mailbox using the integrated microphone and speaker to record and replay voice notes from friends.
- USB-C power workflow for battery-powered operation, serial debugging, charging, and flashing.
- Camera sharing with OV2640 pictures or brief videos sent to friends or the dashboard.
- Four-face display OS with widgets, expressions, friend feeds, time, status, and air data.
- Remote control so friends can control another Cube OS and interact from far away.

## Justification

Some of our team members are graduating from high school and heading to college after 7 years together at the same school, and we thought, why do not we build something to keep our connection more real and maintain our friend group? Social media and group chats alone make it hard to keep the connection physical, and we will not always have time to fly to each other's cities to meet.

With Cube OS, we aim to keep annoying each other like we used to, in a friendly manner, by controlling each other's robots and sending random noises and messages at 2 am, like crashing into each other's dorm rooms as we used to. With its extra features, it can also serve as a handy desktop companion for productivity or just an accessory for your desk.

## BOM Breakdown

| Item | Qty | Estimate |
| --- | ---: | ---: |
| PCB, minimum production amount | 2 | 144 USD |
| 9 g micro servo motors | 2 | 8 USD |
| OV2640 camera | 1 | 5 USD |
| AS02504AR speaker | 1 | 5 USD |
| 1S 3.7 V LiPo battery | 1 | 6 USD |
| ER-TFT1.54-2 display panels | 4 | 24 USD |
| **Total** |  | **192 USD** |

## Current Status Photos

<img src="Media/photos/Mechanical%20status%2020260511222505_905_44.jpg" alt="Mechanical linkage current status" width="420">

Current mechanical linkage prototype.

<img src="Media/photos/PCB%20status%202026-05-11%20at%2010.22.09%E2%80%AFPM.png" alt="PCB current status" width="420">

Current PCB layout and component placement.

## Project Files

Short index of the main current files. Directory paths, placeholder files, metadata files, scaffold code, TODO files, and planning notes are intentionally excluded.

- PCB source: `PCB/Rev A/source/Cube OS/Cube OS.kicad_pro`, `PCB/Rev A/source/Cube OS/Cube OS.kicad_sch`, `PCB/Rev A/source/Cube OS/Cube OS.kicad_pcb`
- Fabrication package: `PCB/Rev A/gerbers/Gerber.zip`, `PCB/Rev A/production/BOM.csv`, `PCB/Rev A/production/CPL.csv`
- PCB 3D export: `PCB/Rev A/3d_models/PCB 3d model.step`
- Order package: `PCB/Rev A/order/BOM_manufacture.xls`, `PCB/Rev A/order/ordering contract.pdf`
- Mechanical assembly: `CAD/Legs/Linkage V2/Assembly/Stepping foot linkage prototype v2.step`
- Current status photos: `Media/photos/Mechanical status 20260511222505_905_44.jpg`, `Media/photos/PCB status 2026-05-11 at 10.22.09 PM.png`

## Placeholder, Scaffold, And Planning Content

These paths are not listed as active project files because they currently contain placeholder files, metadata files, TODO-only files, scaffold code, or planning notes. There is no final project content inside them yet.

- `Media/diagrams/`
- `Media/renders/`
- `Media/demo/`
- `Firmware/`
- `BOM/`
- `Docs/`
- `Software/`
- `Software/src/`
- `Software/screenshots/`
- `Journal/`
- `Journal/images/`
- `Zine/`
- `Zine/source/`

Also excluded from the active project list: `CAD/Legs/Linkage V2/Components/Placeholder Bar.step`, `Docs/assembly_guide.md`, and `Docs/troubleshooting.md`.

## License

MIT. See `LICENSE`.
