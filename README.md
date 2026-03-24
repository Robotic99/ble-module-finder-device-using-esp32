# ESP32 Finder

ESP32 Finder is a BLE-based object finder using an ESP32 and an Android app.

## Overview

The ESP32 works as a BLE device.  
The Android app scans, connects, reads RSSI, and sends commands.  
The device gives feedback using an LED and buzzer.

## Features

- BLE scan and connect
- RSSI-based distance estimate
- Manual ring alert
- Distance ring mode
- LED and buzzer feedback
- Android app in Kotlin
- ESP32 firmware in C++

## Project Structure

```text
esp32-finder/
├── firmware/
│   └── CHIPCODE.cpp
├── android-app/
│   ├── MainActivity.kt
│   └── AndroidManifest.xml
├── docs/
│   └── final-report.pdf
└── README.md
