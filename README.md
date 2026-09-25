# SafeTrail

<img width="1000" height="595" alt="ESP32 DevKit V1 38-pin diagram" src="https://github.com/user-attachments/assets/70127d7a-a3ee-4f65-bf13-512b797dd93b" />

> ESP32 DevKit V1 — 38-pin reference diagram


<img width="3300" height="2550" alt="Insert your Channel Id" src="https://github.com/user-attachments/assets/72e97ac5-16a4-4a7b-ad51-3bad6b4a6d77" />
<img width="3300" height="2550" alt="SafeTrail Dashboard_page-0002" src="https://github.com/user-attachments/assets/8900493d-42db-45d9-8234-acb71f7365ab" />

Webpage view Data will upload in the Thinkspeak a cloud platfrom and then through the private api this page show the movement of the user.


## Components

- ESP32 DevKit V1 (38-pin)
- MPU6050 accelerometer and gyroscope module
- DHT22 temperature and humidity sensor (3-pin module)
- SSD1306 I2C OLED display
- Buzzer module (3-pin)
- 3 × push-button modules: SOS, SAFE, and MODE
- Vibration motor
- NPN transistor for motor switching
- 200 Ω resistor for the transistor base
- Flyback diode for the vibration motor
- NEO-6M GPS module
- SIM800L GSM module
- 1000 µF, 16 V electrolytic capacitor for SIM800L power stabilization
- Li-ion battery (3.7–4.2 V)
- TP4056 Li-ion battery charging/protection module
- 10 kΩ pull-up resistor for the DHT22 data line
- Connecting wires and a common-ground connection

## Circuit Wiring / Pin Connections

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

A **10 kΩ pull-up resistor** must be connected between the DHT22 **VCC** and **DATA/OUT** pins.

### 3. SSD1306 OLED Display — I2C

| OLED Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SCL | GPIO 22 (shared I2C bus with the MPU6050) |
| SDA | GPIO 21 (shared I2C bus with the MPU6050) |

### 4. Buzzer Module

| Buzzer Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| I/O | GPIO 27 |

### 5. Push-Button Modules

| Button | VCC | GND | OUT |
|---|---:|---|---:|
| SOS | 3.3 V | GND | GPIO 32 |
| SAFE | 3.3 V | GND | GPIO 33 |
| MODE | 3.3 V | GND | GPIO 25 |

### 6. Vibration Motor — Transistor Driver Circuit

The vibration motor is powered from the battery through a transistor switch. Do not drive the motor directly from an ESP32 GPIO pin.

```text
ESP32 GPIO 26 ──[200 Ω resistor]── Transistor BASE

Battery (+) ───────────── Motor (+)
Motor (−) ────────────── Transistor COLLECTOR
Transistor EMITTER ───── Common GND
```

- Connect the battery positive terminal to the motor positive terminal.
- Connect the motor negative terminal to the transistor collector.
- Connect the transistor emitter to the common ground shared by the battery and ESP32.
- Connect a flyback diode across the motor terminals, with the diode **cathode connected toward Battery (+)**.

### 7. NEO-6M GPS Module

| GPS Pin | ESP32 Connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| TX | GPIO 16 (ESP32 RX2) |
| RX | GPIO 17 (ESP32 TX2) |

The serial connection is crossed: **GPS TX → ESP32 RX** and **GPS RX → ESP32 TX**.

### 8. SIM800L GSM Module

The SIM800L must be powered directly from the Li-ion battery supply. **Do not power the SIM800L from the ESP32 3.3 V output.**

| SIM800L Pin | Connection |
|---|---|
| VCC | Directly to the battery supply (3.7–4.2 V) |
| GND | Battery GND and common ESP32 GND |
| TX | GPIO 13 (ESP32 RX1) |
| RX | GPIO 14 (ESP32 TX1) |

Connect a **1000 µF, 16 V electrolytic capacitor** directly between the SIM800L **VCC and GND** pins. Observe the capacitor polarity: the positive terminal must connect to VCC and the negative terminal must connect to GND.

The serial connection is crossed: **SIM800L TX → ESP32 RX** and **SIM800L RX → ESP32 TX**.

### 9. Power System

```text
Li-ion Battery (+) ──→ TP4056 IN+
Li-ion Battery (−) ──→ TP4056 IN−

TP4056 OUT+ ──────────→ SIM800L VCC and vibration-motor supply
TP4056 OUT− ──────────→ Common GND
```

## Pin Summary

| Function | ESP32 GPIO / Connection |
|---|---|
| I2C SDA — MPU6050 + OLED | GPIO 21 |
| I2C SCL — MPU6050 + OLED | GPIO 22 |
| DHT22 data | GPIO 4 |
| SOS button | GPIO 32 |
| SAFE button | GPIO 33 |
| MODE button | GPIO 25 |
| Buzzer | GPIO 27 |
| Vibration motor control | GPIO 26 |
| GPS RX2 | GPIO 16 |
| GPS TX2 | GPIO 17 |
| SIM800L RX1 | GPIO 13 |
| SIM800L TX1 | GPIO 14 |
| Wi-Fi | Built into the ESP32; no external wiring required |

## Important Wiring Notes

- All grounds must be connected together: ESP32, battery, GPS, SIM800L, motor-driver circuit, sensors, and OLED.
- A common ground is required for reliable signal communication and serial data transfer.
- Confirm the polarity of the 1000 µF capacitor before powering the SIM800L.
- Keep the SIM800L power wiring short and capable of handling its current peaks.
- Verify the transistor pinout and the voltage requirements of every module before applying power.
