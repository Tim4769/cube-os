# PCB

Cube OS Rev A electrical design, KiCad source, Gerbers, PCBA files, footprints, and linked STEP models.

## Rev A

- Project: `Cube OS`
- KiCad: 9.0
- Board: 60.05 mm x 60.05 mm, 4-layer FR4, 1.599 mm
- Finish: HAL lead-free
- Assembly: top-side SMT and connectors
- Fabrication archive: `Rev A/gerbers/Gerber.zip`

## Active Files

- Source: `Rev A/source/Cube OS/`
- BOM: `Rev A/production/BOM.csv`
- CPL: `Rev A/production/CPL.csv`
- Notes: `Rev A/production/fabrication_notes.md`
- Gerbers: `Rev A/gerbers/Gerber/`
- 3D render exports: `Rev A/3d_models/`
- Template/reference data: `Template/`

## Electronics

| Area | Parts |
| --- | --- |
| MCU | ESP32-S3-WROOM-1-N8R8 |
| Displays | Four display FPCs |
| Camera | OV2640 via Amphenol F32R connector |
| Sensors | SHT45, VEML7700, ISM330DHCX, ICS-43434, 10 kOhm NTC |
| Audio | MAX98357A I2S amplifier, JST speaker connector |
| Motion | Two 3-pin servo headers |
| Power | USB-C, 1S LiPo, BQ24074, TPS63802, LDLN025M25R, TPS7A0212 |
| Controls | JS202011JAQN, SPPJ320600 |

## Production Snapshot

- BOM line items: 38
- CPL top-side placements: 45, excluding H1-H4 mounting holes
- Local footprints: `Rev A/source/Cube OS/Cube_OS_Footprint.pretty/`
- Local STEP models: `Rev A/source/Cube OS/3D models/`
