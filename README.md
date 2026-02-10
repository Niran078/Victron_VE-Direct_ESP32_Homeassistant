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

## Firmware installation

### 1) Make sure ESPHome is installed onto your HA server, details can be found here: https://esphome.io/
### 2) Copy the component folder and its contents into your Homeassistant/esphome folder
### 3) Make sure you have a secrets.yaml file in the Homeassistant/esphome folder, this file should include the following:
        wifi_SSID: "xxxx"
        wifi_password: "xxxx"
### 4) Copy the victron-gateway.yaml file into the Homeassistant.esphome folder. 
### 5) Install ESPHome onto your ESP micropocessor and flash the victron-gateway.yaml file to it.

## Hardware installation

### 1) Connect a wire to a GND of the ESP
### 2) Connect a wire to GPIO16 (or another GPIO pin if you changed the config) 
### 3) Crimp both ends and add a 2.54 female JST connector (See image for the pinout), Use the GND and VE.Direct-TX

<img width="765" height="160" alt="image" src="https://github.com/user-attachments/assets/76f246a6-3686-4008-8bcd-18b3e21613fe" />



