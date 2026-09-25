# SafeTrail
**An Embedded Systems Lab Project**

## Project Overview

SafeTrail is a comprehensive embedded systems project designed to demonstrate real-world IoT and safety device development using the ESP32 microcontroller. The system integrates multiple sensors, wireless communication modules, and a cloud-based web dashboard to track user location and environmental conditions in real-time.

### Key Features

- **Real-time Location Tracking** via NEO-6M GPS module
- **Emergency SOS Alert System** with GSM messaging (SIM800L)
- **Environmental Monitoring** using DHT22 (temperature and humidity) and MPU6050 (motion detection)
- **Smart User Interface** with OLED display and three-button control system
- **Cloud Data Integration** via ThingSpeak API for web-based monitoring
- **Active Safety Feedback** with buzzer and vibration motor alerts
- **Battery Powered** with Li-ion charging and protection system

---

## System Architecture

<img width="1000" height="595" alt="ESP32 DevKit V1 38-pin diagram" src="https://github.com/user-attachments/assets/70127d7a-a3ee-4f65-bf13-512b797dd93b" />

> ESP32 DevKit V1 — 38-pin reference diagram

### Dashboard & Web Interface

<img width="3300" height="2550" alt="SafeTrail Configuration Dashboard" src="https://github.com/user-attachments/assets/72e97ac5-16a4-4a7b-ad51-3bad6b4a6d77" />
<img width="3300" height="821" alt="SafeTrail Dashboard_page-0002 - Edited" src="https://github.com/user-attachments/assets/dcb65a47-d7d7-48ce-bdf9-0d34ed47014d" />

**Dashboard Functionality:**
- Real-time GPS location visualization
- Historical movement tracking
- Environmental data graphs (temperature, humidity, motion)
- Emergency alert logs
- Device status and battery monitoring

Data is transmitted to **ThingSpeak** (cloud platform) via the ESP32 microcontroller built-in 2.4 GHz Wi-Fi (802.11 b/g/n) and retrieved through a private API for display on the web dashboard.

---

## Hardware Components

| Component | Specifications | Purpose |
|-----------|---|---|
| **ESP32 DevKit V1** | 38-pin, dual-core microcontroller | Main processor and WiFi/BLE |
| **MPU6050** | 6-axis accelerometer/gyroscope | Motion and fall detection |
| **DHT22** | Temperature/humidity sensor | Environmental monitoring |
| **SSD1306** | 128×64 I2C OLED display | User interface display |
| **NEO-6M GPS** | U-Blox module | Location tracking |
| **SIM800L GSM** | Quad-band cellular module | GSM/GPRS communication |
| **Buzzer Module** | 3-pin active buzzer | Audio alerts |
| **Vibration Motor** | With transistor driver | Tactile notifications |
| **Push Buttons** | SOS, SAFE, MODE buttons | User input controls |
| **Power Management** | TP4056 + Li-ion battery | Charging and power regulation |
| **Supporting Components** | Resistors, diodes, capacitors | Circuit stabilization |

---

## Circuit Wiring & Pin Connections

### 1. MPU6050 — Accelerometer and Gyroscope

| MPU6050 Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

### 2. DHT22 — Temperature and Humidity Sensor

| DHT22 Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| DATA / OUT | GPIO 4 |

**Note:** A **10 kΩ pull-up resistor** must be connected between DHT22 **VCC** and **DATA/OUT** pins.

### 3. SSD1306 OLED Display — I2C

| OLED Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SCL | GPIO 22 (shared I2C bus with MPU6050) |
| SDA | GPIO 21 (shared I2C bus with MPU6050) |

### 4. Buzzer Module

| Buzzer Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| I/O | GPIO 27 |

### 5. Push-Button Modules

| Button | VCC | GND | OUT |
|---|---|---|---|
| **SOS** (Emergency) | 3.3 V | GND | GPIO 32 |
| **SAFE** (All Clear) | 3.3 V | GND | GPIO 33 |
| **MODE** (Function Select) | 3.3 V | GND | GPIO 25 |

### 6. Vibration Motor — Transistor Driver Circuit

⚠️ **The vibration motor MUST NOT be driven directly from an ESP32 GPIO pin.** Use a transistor-based driver circuit.

```text
ESP32 GPIO 26 ──[200 Ω resistor]── NPN Transistor BASE

Battery (+) ───────────── Motor (+)
Motor (−) ────────────── Transistor COLLECTOR
Transistor EMITTER ───── Common GND
```

**Wiring Steps:**
1. Connect battery positive to motor positive
2. Connect motor negative to transistor collector
3. Connect transistor emitter to common ground (battery and ESP32)
4. Connect **flyback diode** across motor terminals (cathode toward battery +)

### 7. NEO-6M GPS Module

| GPS Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| TX | GPIO 16 (ESP32 RX2) |
| RX | GPIO 17 (ESP32 TX2) |

**Serial Connection:** GPS TX → ESP32 RX2 | GPS RX → ESP32 TX2 (crossed)

### 8. SIM800L GSM Module

⚠️ **CRITICAL:** The SIM800L MUST be powered directly from the Li-ion battery (3.7–4.2 V). **Do NOT use ESP32 3.3 V output.**

| SIM800L Pin | Connection |
|---|---|
| VCC | Directly to battery supply (3.7–4.2 V) |
| GND | Battery GND and common ESP32 GND |
| TX | GPIO 13 (ESP32 RX1) |
| RX | GPIO 14 (ESP32 TX1) |

**Power Stabilization:** Connect a **1000 µF, 16 V electrolytic capacitor** directly between SIM800L **VCC and GND** pins. Observe polarity (+ to VCC, − to GND).

**Serial Connection:** SIM800L TX → ESP32 RX1 | SIM800L RX → ESP32 TX1 (crossed)

### 9. Power System

```text
Li-ion Battery (+) ──→ TP4056 IN+
Li-ion Battery (−) ──→ TP4056 IN−

TP4056 OUT+ ──────────→ SIM800L VCC + Vibration Motor Supply
TP4056 OUT− ──────────→ Common GND
```

---

## ESP32 Pin Summary

| Function | ESP32 GPIO |
|---|---|
| I2C SDA (MPU6050 + OLED) | GPIO 21 |
| I2C SCL (MPU6050 + OLED) | GPIO 22 |
| DHT22 Data | GPIO 4 |
| SOS Button | GPIO 32 |
| SAFE Button | GPIO 33 |
| MODE Button | GPIO 25 |
| Buzzer | GPIO 27 |
| Vibration Motor Control | GPIO 26 |
| GPS RX (RX2) | GPIO 16 |
| GPS TX (TX2) | GPIO 17 |
| SIM800L RX (RX1) | GPIO 13 |
| SIM800L TX (TX1) | GPIO 14 |
| Wi-Fi | Built-in (no external wiring) |

---

## Critical Wiring Guidelines

✓ **Best Practices:**
- Connect all grounds together: ESP32, battery, sensors, GPS, SIM800L, motor circuit, and OLED
- Use a **common ground plane** for reliable signal communication and serial data transfer
- Keep power wiring to SIM800L **short and robust** (capable of handling current peaks up to 2A)
- Double-check all polarities (especially the electrolytic capacitor and Li-ion battery)
- Verify transistor pinout and voltage requirements before power-up
- Test each module individually before final integration

⚠️ **Safety Warnings:**
- Li-ion battery handling requires proper protection (use TP4056 module)
- SIM800L draws significant peak current; use dedicated power supply and capacitor
- Always verify component voltage ratings before application
- Use appropriate wire gauges for high-current paths (battery to SIM800L, motor circuit)

---

## Project Learning Outcomes

This project demonstrates proficiency in:
- **Microcontroller Programming** (ESP32 FreeRTOS development)
- **Hardware Design & Integration** (multi-module sensor fusion)
- **Serial Communication Protocols** (I2C, UART)
- **IoT & Cloud Integration** (ThingSpeak API, GSM communication)
- **Embedded Systems Concepts** (power management, real-time constraints, sensor calibration)
- **PCB Design** (circuit layout and schematic design)

---

## License

This project is part of the Embedded Systems Lab curriculum.

## Author
- **Abid Hasan**
- **SEU CSE**
- **Check out my portfolio [here](https://abidhasan27.me).**

