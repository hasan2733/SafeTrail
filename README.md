# SafeTrail

[![Demo Video](https://img.youtube.com/vi/zX_D5FCz2U8/maxresdefault.jpg)](https://youtu.be/zX_D5FCz2U8)

> Demo video: SafeTrail in action

<img width="1000" height="595" alt="ESP32 DevKit V1 38-pin diagram" src="https://github.com/user-attachments/assets/70127d7a-a3ee-4f65-bf13-512b797dd93b" />

> ESP32 DevKit V1 — 38-pin reference diagram

<img width="3300" height="2550" alt="ThingSpeak channel configuration" src="https://github.com/user-attachments/assets/72e97ac5-16a4-4a7b-ad51-3bad6b4a6d77" />
<img width="3300" height="821" alt="SafeTrail dashboard" src="https://github.com/user-attachments/assets/204edfba-201b-4836-b73e-15e86bf4d2f0" />

> Dashboard view: sensor data is uploaded to ThingSpeak and displayed through a private API so the user’s movement and safety status can be monitored in real time.

SafeTrail is a personal safety and emergency-response wearable built around the ESP32 microcontroller. It combines environmental sensing, motion analysis, GPS tracking, geofencing, emergency alerting, and cloud monitoring in a compact embedded system.

The current implementation includes a K-Nearest Neighbors (KNN) risk classifier, geofence logic, OLED status feedback, SMS fallback alerts, and ThingSpeak-based cloud monitoring.

## Overview

SafeTrail continuously monitors:

- Motion and impact using the MPU6050
- Temperature and humidity using the DHT22
- Location using a GPS module
- User safety-zone status using geofencing
- Emergency conditions using KNN-based risk classification

When a risky state is detected or the user triggers SOS, the system can:

- Display the current state on the OLED screen
- Trigger buzzer and vibration alerts
- Send an SMS through the SIM800L
- Upload location, risk, temperature, and humidity data to ThingSpeak

---

## Key Features

- ESP32-based embedded safety system
- MPU6050 accelerometer and gyroscope for motion and fall detection
- DHT22 sensor for temperature and humidity monitoring
- GPS tracking with real-time geofence checking
- KNN risk classifier with five features and four safety states
- OLED UI with multiple display themes
- Three-button interface: SOS, SAFE, and THEME
- Buzzer and vibration alert system
- SIM800L SMS emergency-alert fallback
- Wi-Fi and ThingSpeak cloud upload
- Serial console for testing and debugging without hardware dependence

---

## System Architecture

The SafeTrail firmware is organized around these core blocks:

1. **Sensing Layer**
   - MPU6050: acceleration and angular velocity
   - DHT22: temperature and humidity
   - GPS: latitude and longitude

2. **Decision Layer**
   - KNN risk classifier
   - Geofence distance check
   - Emergency confirmation logic

3. **Response Layer**
   - OLED UI updates
   - Buzzer and vibration alarm
   - SMS via SIM800L
   - Cloud upload through ThingSpeak

4. **Debug and Control Layer**
   - Serial command console
   - Live zone changes
   - Fake GPS injection for testing
   - Sensor-testing commands

---

## Hardware Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| ESP32 DevKit V1 | 38-pin dual-core MCU | Main controller |
| MPU6050 | 6-axis accelerometer/gyroscope | Motion and impact detection |
| DHT22 | Temperature and humidity sensor | Environmental monitoring |
| SSD1306 OLED | 128x64 I2C display | UI and status display |
| GPS module | NEO-6M-style UART GPS | Location tracking |
| SIM800L GSM | GSM/GPRS module | Emergency SMS fallback |
| Buzzer | Passive buzzer | Alert sound |
| Vibration motor | DC motor with transistor driver | Haptic alert |
| Buttons | SOS / SAFE / THEME | User input |
| Power | Li-ion battery and TP4056 | System power |

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
| Vibration motor control | GPIO 26 |
| GPS RX | GPIO 16 |
| GPS TX | GPIO 17 |
| SIM800L RX | GPIO 13 |
| SIM800L TX | GPIO 14 |

### Important Notes

- The MPU6050 and OLED share the same I2C bus: SDA=21, SCL=22.
- The DHT22 requires a 10k pull-up resistor between VCC and DATA.
- The GPS is connected through UART2.
- The SIM800L should be powered directly from the Li-ion battery, not from the ESP32 3.3V rail.
- The vibration motor must use a transistor driver circuit and must not be connected directly to GPIO 26.

---

## KNN Risk Classifier

The firmware includes a working KNN classifier designed to estimate user risk from five normalized features:

- Accelerometer magnitude
- Gyroscope magnitude
- Temperature
- Humidity
- Distance from the safe zone

Labels used in the model:

- `0` = SAFE
- `1` = CAUTION
- `2` = WARNING
- `3` = EMERGENCY

### KNN Configuration

- Number of samples: 25
- Number of features: 5
- K value: 5
- Distance metric: Euclidean distance
- Voting method: Majority vote among the nearest neighbors

### Prediction Flow

1. Read sensor values.
2. Normalize the query feature vector.
3. Normalize the training dataset.
4. Compute Euclidean distances.
5. Sort by closest neighbors.
6. Count votes among the top K entries.
7. Select the label with the highest vote count.

The code includes a verbose KNN test command that displays nearest neighbors and vote counts in the serial console.

### Important Note

The training data in the current code is a placeholder dataset intended for prototype validation. For real-world deployment, it should be replaced with field-collected and labeled user data to improve classification accuracy and reliability.

---

## Geofencing Logic

SafeTrail tracks a safe-zone center point and radius:

- `SAFE_ZONE_LAT`
- `SAFE_ZONE_LNG`
- `SAFE_ZONE_RADIUS_M`

The firmware uses the Haversine formula to calculate the current distance from the safe zone and updates the user state when the device exits or re-enters the protection radius.

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

This runs the KNN test with custom values and prints the nearest neighbors and vote breakdown.

---

## Safety States

The system uses four safety states:

- SAFE
- CAUTION
- WARNING
- EMERGENCY

Emergency behavior can be triggered by:

- Repeated SOS presses
- Sustained KNN emergency detection
- Geofence-breach conditions
- Manual emergency confirmation flow

The SAFE button clears the emergency state after confirmation.

---

## Cloud Integration

The firmware uploads data to ThingSpeak using ESP32 Wi-Fi.

Uploaded fields include:

- GPS latitude
- GPS longitude
- Risk score
- Temperature
- Humidity
- Safety state

This allows real-time monitoring from a web dashboard or other cloud interface.

---

## Libraries Required

The project uses these libraries:

- `WiFi.h`
- `HTTPClient.h`
- `Wire.h`
- `Adafruit_MPU6050`
- `Adafruit_Sensor`
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `DHT sensor library`
- `TinyGPS++`

---

## Setup Instructions

1. Install the ESP32 board package in the Arduino IDE.
2. Install the required libraries listed above.
3. Configure Wi-Fi credentials in the firmware:
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
4. Configure the ThingSpeak API key:
   - `THINGSPEAK_API_KEY`
5. Configure the emergency contact number:
   - `GUARDIAN_PHONE`
6. Set the safe-zone center and radius in the firmware if needed.
7. Verify all wiring and power connections.
8. Upload the sketch and open the Serial Monitor at 115200 baud.

> Do not commit Wi-Fi passwords, API keys, or phone numbers to a public repository. Use a local configuration file or another secure method for deployment credentials.

---

## Operational Behavior

During runtime, SafeTrail:

- Reads sensor values periodically
- Calculates risk using KNN
- Checks geofence distance
- Updates the OLED display
- Manages emergency-state transitions
- Sends SMS alerts when required
- Uploads cloud data at configured intervals

---

## Troubleshooting Tips

### Wi-Fi not connecting

- Check the SSID and password.
- Ensure that 2.4 GHz Wi-Fi is available.
- Confirm that the ESP32 has a stable power supply.

### GPS fix missing

- Verify TX/RX wiring.
- Check the serial configuration and power supply.
- Test with the `GPS` injection command during development.

### SIM800L not sending SMS

- Ensure the SIM card is active and a GSM network is available.
- Verify direct power from the battery.
- Confirm a common ground between the battery and ESP32.
- Inspect the serial TX/RX wiring.

### KNN output seems unstable

- Improve or retune the training-data boundaries.
- Check sensor calibration.
- Use the `KNN` serial command for debugging.

---

## Project Notes

This repository is intended as an embedded-systems safety prototype and demonstration project. The current code supports testing, simulation, and hardware validation. Some components, especially the KNN training data and emergency workflows, should be tested with real field data and reviewed carefully before use in a production safety system.

---

## License

This project is part of the Embedded Systems Lab curriculum.

---

## Author

- Abid Hasan
- SEU CSE
- Portfolio: https://abidhasan27.me
