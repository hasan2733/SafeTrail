# SafeTrail

<img width="1000" height="595" alt="ESP32 DevKit V1 38-pin diagram" src="https://github.com/user-attachments/assets/70127d7a-a3ee-4f65-bf13-512b797dd93b" />

> ESP32 DevKit V1 — 38-pin reference diagram

<img width="3300" height="2550" alt="Insert your Channel Id" src="https://github.com/user-attachments/assets/72e97ac5-16a4-4a7b-ad51-3bad6b4a6d77" />
<img width="3300" height="821" alt="SafeTrail Dashboard_page-0002 - Edited" src="https://github.com/user-attachments/assets/204edfba-201b-4836-b73e-15e86bf4d2f0" />


Webpage view: data is uploaded to ThingSpeak, a cloud platform, and then displayed through a private API so the movement of the user can be monitored in real time.

SafeTrail is a personal safety and emergency response wearable built around the ESP32 microcontroller. It combines environmental sensing, motion analysis, GPS tracking, geofencing, emergency alerting, and cloud-based monitoring to support rapid response in risky conditions.

The current implementation includes a K-Nearest Neighbors (KNN) risk classifier, geofence logic, OLED display status feedback, SMS fallback alerts, and ThingSpeak-based cloud monitoring.

## Overview

SafeTrail continuously monitors:
- motion and impact using MPU6050
- temperature and humidity using DHT22
- location using GPS
- user safety zone status using geofencing
- emergency conditions using KNN-based risk classification

When a risky state is detected or the user triggers SOS, the system can:
- display a state on the OLED screen
- trigger buzzer and vibration alerts
- send an SMS through SIM800L
- upload location, risk, temperature, and humidity data to ThingSpeak

---

## Key Features

- ESP32-based embedded safety system
- MPU6050 accelerometer + gyroscope for motion and fall detection
- DHT22 sensor for temperature and humidity monitoring
- GPS tracking with real-time geofence checking
- KNN risk classifier with 5 features and 4 safety states
- OLED UI with multiple display themes
- 3-button interface: SOS, SAFE, THEME
- Buzzer + vibration alert system
- SIM800L SMS emergency alert fallback
- WiFi + ThingSpeak cloud upload
- Serial console for testing and debugging without hardware dependence

---

## System Architecture

The SafeTrail firmware is organized around these core blocks:

1. Sensing Layer
   - MPU6050: acceleration and angular velocity
   - DHT22: temperature and humidity
   - GPS: latitude and longitude

2. Decision Layer
   - KNN risk classifier
   - geofence distance check
   - emergency confirmation logic

3. Response Layer
   - OLED UI updates
   - buzzer/vibration alarm
   - SMS via SIM800L
   - cloud upload through ThingSpeak

4. Debug/Control Layer
   - Serial command console
   - live zone changes
   - fake GPS injection for testing
   - sensor testing commands

---

## Hardware Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| ESP32 DevKit V1 | 38-pin dual-core MCU | Main controller |
| MPU6050 | 6-axis accel/gyro | Motion and impact detection |
| DHT22 | Temperature + humidity | Environmental monitoring |
| SSD1306 OLED | 128x64 I2C display | UI/status display |
| GPS Module | NEO-6M style UART GPS | Location tracking |
| SIM800L GSM | GSM/GPRS module | Emergency SMS fallback |
| Buzzer | Passive buzzer | Alert sound |
| Vibration Motor | DC motor + transistor driver | Haptic alert |
| Buttons | SOS / SAFE / THEME | User input |
| Power | Li-ion battery + TP4056 | System power |

---

## Actual Wiring Summary

This project matches the current firmware in `SafeTrail.ino`.

### Sensor and I/O Connections

| Device | ESP32 Pin |
|--------|-----------|
| MPU6050 SDA | GPIO 21 |
| MPU6050 SCL | GPIO 22 |
| DHT22 DATA | GPIO 4 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| SOS Button | GPIO 32 |
| SAFE Button | GPIO 33 |
| THEME Button | GPIO 25 |
| Buzzer | GPIO 27 |
| Vibration Motor Control | GPIO 26 |
| GPS RX | GPIO 16 |
| GPS TX | GPIO 17 |
| SIM800L RX | GPIO 13 |
| SIM800L TX | GPIO 14 |

### Important Notes
- The MPU6050 and OLED share the same I2C bus: SDA=21, SCL=22
- DHT22 requires a 10k pull-up resistor between VCC and DATA
- GPS is connected via UART2
- SIM800L should be powered directly from the Li-ion battery, not from the ESP32 3.3V rail
- The vibration motor must use a transistor driver circuit, not directly from GPIO 26

---

## KNN Risk Classifier

The firmware includes a working KNN classifier designed to estimate user risk based on five normalized features:

- accel magnitude
- gyro magnitude
- temperature
- humidity
- distance from safe zone

Labels used in the model:
- 0 = SAFE
- 1 = CAUTION
- 2 = WARNING
- 3 = EMERGENCY

### KNN Configuration
- Number of samples: 25
- Number of features: 5
- K value: 5
- Distance metric: Euclidean distance
- Voting method: majority vote among nearest neighbors

### Prediction Flow
1. Read sensor values
2. Normalize the query feature vector
3. Normalize the training dataset
4. Compute Euclidean distances
5. Sort by closest neighbors
6. Count votes among the top K entries
7. Select the label with the highest vote count

The code includes a verbose KNN test command to inspect nearest neighbors and vote counts directly from the serial console.

### Important Note
The training data in the current code is a placeholder dataset intended for prototype validation. For real deployment, it should be replaced with field-collected and labeled user data for better accuracy and reliability.

---

## Geofencing Logic

SafeTrail tracks a safe-zone center point and radius:
- `SAFE_ZONE_LAT`
- `SAFE_ZONE_LNG`
- `SAFE_ZONE_RADIUS_M`

The firmware uses the Haversine formula to calculate the current distance from the safe zone and updates the user state if the device exits or re-enters the protection radius.

This helps detect when the user has moved outside the defined safe region.

---

## Serial Console Commands

The project includes a serial test console for debugging and validation.

Available commands in the firmware:

```text
HELP
STATUS
GPS <lat> <lng>
ZONE <lat> <lng> <radius_m>
KNN <accel> <gyro> <temp> <hum> <dist>
SOS
SAFEBTN
THEME
CLOUD <seconds>
WIFI
RAWACCEL
DHTTEST
MPUTEST
OPMODE <SAFE|TRAVEL|SILENT|EMERGENCY|TEST>
```

### Example
```text
KNN 12.5 1.2 29.5 60 180
```
This runs the KNN test using custom values and prints nearest neighbors and vote breakdown.

---

## Safety States

The system uses four safety states:

- SAFE
- CAUTION
- WARNING
- EMERGENCY

Emergency behavior can be triggered by:
- repeated SOS presses
- sustained KNN emergency detection
- geofence breach conditions
- manual emergency confirmation flow

The SAFE button clears emergency state after confirmation.

---

## Cloud Integration

The firmware uploads data to ThingSpeak using ESP32 WiFi.

Uploaded fields include:
- GPS latitude
- GPS longitude
- risk score
- temperature
- humidity
- safety state

This allows real-time monitoring from a web dashboard or cloud interface.

---

## Libraries Required

The project uses these libraries:

- WiFi.h
- HTTPClient.h
- Wire.h
- Adafruit_MPU6050
- Adafruit_Sensor
- Adafruit_GFX
- Adafruit_SSD1306
- DHT sensor library
- TinyGPS++

---

## Setup Instructions

1. Install the ESP32 board package in Arduino IDE
2. Install the required libraries listed above
3. Configure WiFi credentials in the firmware:
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
4. Configure the ThingSpeak API key:
   - `THINGSPEAK_API_KEY`
5. Configure the emergency contact number:
   - `GUARDIAN_PHONE`
6. Set the safe zone center and radius in the firmware if needed
7. Verify all wiring and power connections
8. Upload the sketch and open the Serial Monitor at 115200 baud

---

## Operational Behavior

During runtime, SafeTrail does the following:
- reads sensor values periodically
- calculates risk using KNN
- checks geofence distance
- updates OLED display
- manages emergency state transitions
- sends SMS if needed
- uploads cloud data at configured intervals

---

## Troubleshooting Tips

### WiFi not connecting
- check SSID/password
- ensure 2.4GHz WiFi is available
- confirm the ESP32 has good power supply

### GPS fix missing
- verify TX/RX wiring
- check serial config and power
- test with `GPS` injection command during development

### SIM800L not sending SMS
- ensure SIM card is active and GSM network available
- verify direct power from battery
- confirm common ground between battery and ESP32
- inspect serial TX/RX wiring

### KNN output seems unstable
- tune training data boundaries
- check sensor calibration
- use `KNN` serial command for debugging

---

## Project Notes

This repository is intended as an embedded systems safety prototype and demonstration project. The current code is functional for testing, simulation, and hardware validation, but some parts, especially the deployed risk model and communication configuration, should be refined before real-world deployment.

---

## License

This project is part of the Embedded Systems Lab curriculum.

---

## Author

- Abid Hasan
- SEU CSE
- Portfolio: https://abidhasan27.me
