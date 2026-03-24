# ESP32 Finder / Opti-Find

Opti-Find is a Bluetooth Low Energy (BLE) based nearby object locator built using an ESP32 module and an Android application.  
It is designed for short-range indoor object finding, where the user knows the item is nearby but cannot immediately see it.

The system uses BLE communication between an Android phone and an ESP32 device. The app scans and connects to the device, reads live RSSI values, smooths them using filtering, estimates proximity bands, and sends commands to trigger LED and buzzer alerts. :contentReference[oaicite:4]{index=4} :contentReference[oaicite:5]{index=5}

---

## Features

- BLE-based discovery and connection using a custom service UUID
- ESP32 acts as BLE peripheral / GATT server
- Android app acts as BLE central / GATT client
- Live RSSI monitoring for nearby distance estimation
- RSSI smoothing using:
  - Exponential Moving Average (EMA)
  - 1-D Kalman Filter
- Proximity band classification:
  - FAR
  - MID
  - NEAR
  - VERY_CLOSE
- Trend detection:
  - Getting closer
  - Getting farther
  - About the same
- Manual ring alert
- Adaptive distance ring mode
- LED and buzzer-based local feedback
- Android app UI built with Jetpack Compose 

---

## System Overview

The project contains two main parts:

### 1. ESP32 Locator Device
The ESP32 advertises a custom BLE service and accepts commands from the Android application.  
It controls the local outputs such as buzzer and LED for locate feedback. 

### 2. Android Mobile Application
The Android app scans for the ESP32 device, connects over BLE, discovers GATT services, writes commands, receives notifications, and displays RSSI-based feedback to the user.  
The app is built in Kotlin using Jetpack Compose. 

---

## BLE Architecture

### Service UUID
`12345678-1234-1234-1234-1234567890ab`

### Characteristics
- **Control Characteristic**  
  `12345678-1234-1234-1234-1234567890ac`
- **Status Characteristic**  
  `12345678-1234-1234-1234-1234567890ad`

The Android app writes commands to the control characteristic and receives status notifications from the status characteristic. 

---

## Current Command Support

### Confirmed in ESP32 firmware
- `PING`
- `ALERT=ON`
- `ALERT=OFF`
- `RANGE=FAR`
- `RANGE=MID`
- `RANGE=NEAR`
- `RANGE=VERY_CLOSE`
- `RANGE=IDLE` 

### Present in Android app flow / report
- `LED=ON`
- `LED=OFF`
- `TONE=1`
- `TONE=2`
- `TONE=3`
- `TONE=OFF` 

> Note: the Android app and report mention LED and custom tone commands, but the uploaded ESP32 firmware currently shows direct handling only for `PING`, `ALERT=ON/OFF`, and `RANGE=...` commands. The firmware may need to be updated so both sides fully match. 

---

## RSSI-Based Distance Estimation

The Android app reads BLE RSSI values and smooths them before presenting them to the user.

### Supported filters
- **EMA**
  - Alpha = `0.25`
- **Kalman Filter**
  - Lightweight 1-D implementation for mobile execution 

### Proximity bands
- **FAR** for RSSI below `-80 dBm`
- **MID** for RSSI between `-80` and `-70 dBm`
- **NEAR** for RSSI between `-70` and `-60 dBm`
- **VERY_CLOSE** for RSSI stronger than `-60 dBm` 

---

## Hardware Requirements

- ESP32 development board
- Buzzer
- LED
- USB power source
- Breadboard / jumper wires
- Enclosure or casing
- Android phone with BLE support :contentReference[oaicite:15]{index=15}

---

## Software Requirements

- Arduino IDE / ESP32 firmware toolchain
- Android Studio
- Kotlin
- Jetpack Compose
- Android BLE APIs
- BLE permissions on Android:
  - `BLUETOOTH`
  - `BLUETOOTH_ADMIN`
  - `BLUETOOTH_SCAN`
  - `BLUETOOTH_CONNECT`
  - `ACCESS_FINE_LOCATION` 

---

## Android Permissions

The Android manifest includes BLE-related permissions required for scanning and communication. :contentReference[oaicite:17]{index=17}

---

## Project Structure

Suggested repository structure:

```text
esp32-finder/
│
├── firmware/
│   └── CHIPCODE.cpp
│
├── android-app/
│   ├── MainActivity.kt
│   └── AndroidManifest.xml
│
├── docs/
│   └── final-report.pdf
│
└── README.md
