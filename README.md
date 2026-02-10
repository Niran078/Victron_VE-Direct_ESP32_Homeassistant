# Victron VE.Direct → ESPHome → Home Assistant (ESP32, Wi-Fi)

Read Victron SmartSolar MPPT data over **VE.Direct (text mode)** with an **ESP32** and publish it into **Home Assistant** via **ESPHome** (Wi-Fi).  
Includes: checksum validation, data-valid watchdog, decoded state/off-reason, and error-code text decoding.

## Features
- Read-only VE.Direct text protocol (**19200 8N1**) with checksum validation
- Sensors:
  - Battery voltage/current
  - Panel voltage/power
  - Load current
  - Load output state (ON/OFF)
  - Off reason (raw + text)
  - State of operation (CS) (raw + text)
  - Error code (ERR) (raw + decoded text)
  - Max power today (H21)
  - Yield today (H20)
  - Yield total (H19)
  - Tracker operation mode (MPPT) (raw + text)
- Diagnostics:
  - Data valid boolean
  - Frame age (seconds)
  - OK/Bad frame counters

## Hardware
- ESP32 (tested with ESP32-WROOM / esp32dev)
- Victron SmartSolar MPPT with VE.Direct port (e.g. 100/20)
- 2 wires minimum:
  - Victron **GND** → ESP32 **GND**
  - Victron **TX** → ESP32 **GPIO16** (RX)


## Wiring (VE.Direct)
Typical VE.Direct pinout (check your cable/device):
- GND → ESP32 GND
- TX  → ESP32 GPIO16

No connection is required to Victron RX (this project is **read-only**).

## Installation

### 1) Copy the external component into ESPHome
In your Home Assistant config directory:

