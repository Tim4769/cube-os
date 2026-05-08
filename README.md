# Cube OS

Fallout/Burnout-style hardware project repository scaffold.

> Add the main project render at `Media/renders/main_render.png` before review.

## Overview

Cube OS is a hardware project workspace for organizing CAD, PCB, firmware, documentation, build media, and project journal notes in a reviewer-friendly layout.

## Why I Made This

TODO: Explain the personal motivation for the build. Make this specific: what problem it solves, what inspired the visual or interaction style, and what you learned while making it.

## Features

- TODO: Feature 1
- TODO: Feature 2
- TODO: Feature 3
- TODO: Feature 4

## Hardware

Main components:

- MCU: TODO
- Sensors: TODO
- Displays: TODO
- Motors/servos: TODO
- Battery/power: TODO
- PCB: TODO

## CAD

The full assembly STEP file should live here:

`CAD/assembly.step`

CAD source files are in:

`CAD/source/`

Printable files are in:

`CAD/print_files/`

## PCB

KiCad source files are in:

`PCB/source/`

Gerbers should be exported to:

`PCB/gerbers/CubeOS_gerbers.zip`

Schematic PDF should be exported to:

`PCB/schematic.pdf`

## Firmware

Firmware is in:

`Firmware/`

To build/upload with PlatformIO:

```bash
pio run
pio run --target upload
```

## BOM

The BOM is available here:

`BOM/bom.csv`

| Item | Qty | Cost | Link | Notes |
| --- | --: | ---: | --- | --- |
| ESP32-S3 | 1 | $4 | TODO | Main MCU placeholder |

Total estimated cost: **$XX**

## Assembly

See:

`Docs/assembly_guide.md`

Basic steps:

1. Order PCB.
2. Print case.
3. Solder components.
4. Mount PCB into case.
5. Flash firmware.
6. Test.

## Usage

TODO: Explain how to use the finished project.

## Testing

TODO: Explain what has been tested and what still needs testing.

## Images

Add renders, PCB screenshots, CAD screenshots, and build photos under:

- `Media/renders/`
- `Media/photos/`
- `Media/diagrams/`
- `CAD/renders/`
- `PCB/3d_models/`

## License

This project is licensed under the MIT License. See `LICENSE`.
