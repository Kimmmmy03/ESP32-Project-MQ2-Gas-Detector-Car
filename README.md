<div align="center">

<img src="https://img.shields.io/badge/Platform-ESP32-blue?style=flat-square&logo=espressif" />
<img src="https://img.shields.io/badge/Language-Arduino%20C%2B%2B-teal?style=flat-square&logo=arduino" />
<img src="https://img.shields.io/badge/Status-Completed-success?style=flat-square" />
<img src="https://img.shields.io/badge/Version-1.0.0-informational?style=flat-square" />
<img src="https://img.shields.io/badge/License-Academic-lightgrey?style=flat-square" />

# Automated MQ-2 Gas Detector Car

**ISB37503 — Realtime and Embedded System**  
Universiti Kuala Lumpur (UniKL) · December 2025

*An autonomous robot car that continuously patrols enclosed spaces, detects hazardous gas concentrations in real-time, avoids obstacles in 360°, and triggers an immediate alarm — without any human intervention.*

</div>

---

## Table of Contents

- [Overview](#overview)
- [Gallery](#gallery)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Pin Mapping](#pin-mapping)
- [Software Design](#software-design)
- [Getting Started](#getting-started)
- [Calibration](#calibration)
- [Test Results](#test-results)
- [Project Budget](#project-budget)
- [Team](#team)
- [License](#license)

---

## Overview

Gas leaks and smoke accumulation in enclosed environments — kitchens, laboratories, workshops, and storage rooms — represent serious safety hazards that often go undetected until it is too late. Stationary gas sensors are limited to a fixed detection radius, leaving large portions of a room unmonitored.

This project addresses that gap with a **fully autonomous mobile gas detection platform** built on the ESP32 microcontroller. The robot continuously roams its environment, sampling air quality every 100 ms. When a hazardous gas concentration is detected, all motors stop immediately and a buzzer alarm activates — alerting nearby personnel in under one second.

### Key Capabilities

| Capability | Detail |
|---|---|
| Gas detection | MQ-2 sensor — LPG, methane, propane, hydrogen, CO, smoke |
| Detection threshold | ADC value ≥ 3200 (calibrated against real-world lighter gas) |
| Alarm response time | < 1 second from threshold crossing to buzzer + motor stop |
| Obstacle avoidance | 4× HC-SR04 ultrasonic sensors covering all four quadrants |
| Obstacle threshold | < 20 cm triggers avoidance manoeuvre |
| Power source | 2× 18650 Li-ion cells (2200 mAh) — fully autonomous |
| Processing unit | ESP32 DevKitC V4 — dual-core, 240 MHz |

---

## Gallery
Front 
<img width="373" height="308" alt="Screenshot 2026-04-26 184601" src="https://github.com/user-attachments/assets/d802bd8e-be75-4957-8d7b-88bdb426f839" />
Rear
<img width="386" height="391" alt="Screenshot 2026-04-26 184554" src="https://github.com/user-attachments/assets/5cff057c-6955-4d8b-950c-e9a0ed64dcee" />
Left
<img width="577" height="426" alt="Screenshot 2026-04-26 184545" src="https://github.com/user-attachments/assets/ba4d62da-8d76-4cc1-8b8c-66cdb0195596" />
Right
<img width="573" height="432" alt="Screenshot 2026-04-26 184531" src="https://github.com/user-attachments/assets/deda7453-55ec-4729-b8ec-fe020a431832" />
---

## System Architecture

### Block Diagram

```
┌───────────────────────────────────────────────────────────┐
│                       INPUT LAYER                         │
│                                                           │
│   MQ-2 Gas Sensor              4× HC-SR04 Ultrasonic      │
│   Analog · GPIO 34             Digital · 8 GPIOs          │
└─────────────────────┬─────────────────────────────────────┘
                      │
┌─────────────────────▼─────────────────────────────────────┐
│                   PROCESSING LAYER                        │
│                                                           │
│                  ESP32 DevKitC V4                         │
│     Reads MQ-2 analog value every 100 ms                  │
│     Fires ultrasonic pulses, measures echo time           │
│     Executes priority-based decision logic                │
│     Streams debug data to Serial Monitor                  │
└──────────┬──────────────────────────┬─────────────────────┘
           │                          │
┌──────────▼──────────┐  ┌────────────▼────────────────────┐
│    ALERT LAYER      │  │        ACTUATION LAYER          │
│                     │  │                                 │
│  Buzzer · GPIO 27   │  │  L298N Motor Driver             │
│  Active HIGH        │  │  ├── Left DC Motor  (ENA/IN1/2) │
│  Sounds on gas      │  │  └── Right DC Motor (ENB/IN3/4) │
└─────────────────────┘  └─────────────────────────────────┘
```

### Control Logic

Gas detection holds the **highest priority** in the control loop. Obstacle avoidance and roaming are only executed when no gas is present.

```
LOOP (every ~100 ms)
│
├── Read all 4 ultrasonic distances
│
├── [PRIORITY 1] Gas detected? (MQ-2 ≥ 3200)
│       YES  →  Stop all motors + Buzzer ON → return
│       NO   →  Buzzer OFF → continue
│
├── [PRIORITY 2] Obstacle within 20 cm?
│       Front-Left only        →  Turn Right
│       Front-Right only       →  Turn Left
│       Both Front blocked     →  Reverse        (rear clear)
│                              →  Rotate in place (rear blocked)
│       Rear only              →  Continue Forward
│
└── [PRIORITY 3] Path clear → Drive Forward (default roaming)
```

### Circuit Diagram

<img width="583" height="623" alt="Screenshot 2026-04-26 184519" src="https://github.com/user-attachments/assets/338ceeb5-f52e-49d9-a5ba-6359f405b2fc" />

---

## Hardware Components

### Sensors & Input

| # | Component | Qty | Function |
|---|-----------|:---:|---------|
| 1 | MQ-2 Gas Sensor | 1 | Detects combustible gases and smoke via analog voltage output |
| 2 | HC-SR04 Ultrasonic Sensor | 4 | Measures echo time to calculate distance to obstacles in four directions |

### Processing

| # | Component | Qty | Function |
|---|-----------|:---:|---------|
| 1 | ESP32 DevKitC V4 (38-pin) | 1 | Central MCU — sensor input, decision logic, PWM output, Serial debug |

### Output & Actuation

| # | Component | Qty | Function |
|---|-----------|:---:|---------|
| 1 | L298N Motor Driver Module | 1 | H-bridge driver — bidirectional speed and direction control via PWM |
| 2 | DC Gear Motor | 2 | Drive wheels — forward, reverse, and differential turning |
| 3 | Buzzer (3–24 V DC) | 1 | Audible alarm triggered on gas detection event |

### Power Supply

| # | Component | Qty | Function |
|---|-----------|:---:|---------|
| 1 | 18650 Li-ion Battery 2200 mAh | 2 | Primary portable power — no external supply required |
| 2 | 18650 Battery Holder (2-slot) | 2 | Secure battery mounting |
| 3 | 18650 Battery Charger | 1 | Recharging between sessions |

### Mechanical Structure

| # | Component | Qty | Function |
|---|-----------|:---:|---------|
| 1 | 2WD Compact Robot Car Chassis | 1 | Structural base — mounts all hardware |
| 2 | Drive Wheels + Castor Wheel | — | Mobility, balance, and directional control |
| 3 | 3D-Printed Ultrasonic Bracket | 4 | Positions HC-SR04 sensors at correct angles |

---

## Pin Mapping

### Gas Sensor & Alert

| Signal | GPIO | Type | Notes |
|--------|:----:|:----:|-------|
| MQ-2 analog out | 34 | INPUT (ADC) | `ADC_11db` attenuation — full 0–3.3 V range |
| Buzzer | 27 | OUTPUT | Active HIGH |

### L298N Motor Driver

| Signal | GPIO | Direction |
|--------|:----:|:---------:|
| IN1 — Left motor A | 13 | OUTPUT |
| IN2 — Left motor B | 12 | OUTPUT |
| IN3 — Right motor A | 15 | OUTPUT |
| IN4 — Right motor B | 14 | OUTPUT |
| ENA — Left speed (PWM) | 26 | OUTPUT |
| ENB — Right speed (PWM) | 25 | OUTPUT |

### HC-SR04 Ultrasonic Sensors

| Position | TRIG GPIO | ECHO GPIO |
|----------|:---------:|:---------:|
| Front-Left | 32 | 33 |
| Front-Right | 2 | 35 |
| Rear-Left | 16 | 17 |
| Rear-Right | 4 | 5 |

---

## Software Design

### Language & Environment

- **Language:** Arduino/C++ targeting ESP32
- **IDE:** Arduino IDE v1.8+ or v2.x
- **External libraries:** None required — only built-in Arduino ESP32 APIs

### Firmware Structure

```
gas_detector_car.ino
│
├── Global definitions        Pin constants, threshold values, PWM config
├── measureDistance()         HC-SR04 trigger → echo timing → distance in cm
├── setMotorA / setMotorB()   Low-level PWM + direction control per motor
├── stopMotors()              Zero all motors immediately
├── driveForward()            Both motors forward at DRIVE_SPEED
├── driveBackward()           Both motors reverse at DRIVE_SPEED
├── turnLeft()                Differential steering — pivot left in place
├── turnRight()               Differential steering — pivot right in place
├── gasDetected()             analogRead + threshold comparison + Serial log
├── setup()                   Pin modes, PWM channels, safe startup state
└── loop()                    Main control loop — runs every ~100 ms
```

### Key Constants

```cpp
const int GAS_THRESHOLD     = 3200;   // MQ-2 ADC value (0–4095)
const int OBSTACLE_DISTANCE = 20;     // centimetres
const int DRIVE_SPEED       = 200;    // PWM duty cycle (0–255)
```

### PWM Configuration (ESP32 LEDC)

```cpp
const int pwmChannelA   = 0;
const int pwmChannelB   = 1;
const int pwmFreq       = 1000;   // 1 kHz
const int pwmResolution = 8;      // 8-bit resolution (0–255)
```

---

## Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) v1.8 or later
- ESP32 board support package — [installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- USB-A to Micro-USB cable
- Assembled hardware per the circuit diagram above

### Installation

**1. Clone the repository**

```bash
git clone https://github.com/YOUR_USERNAME/mq2-gas-detector-car.git
cd mq2-gas-detector-car
```

**2. Open the firmware**

Open `gas_detector_car.ino` in the Arduino IDE.

**3. Select the board and port**

```
Tools → Board  →  ESP32 Arduino  →  ESP32 Dev Module
Tools → Port   →  COMx (Windows)  /  /dev/ttyUSB0 (Linux/macOS)
```

**4. Upload**

Click the **→ Upload** button and wait for `Done uploading.`

**5. Open Serial Monitor**

`Tools → Serial Monitor` — set baud rate to **115200**.

Expected output in clean air:

```
=== Gas Detector Car Initialized ===
Gas sensor value: 2641 | Threshold: 3200
FL: 999  FR: 999  RL: 999  RR: 999
Path clear → moving forward
```

### Repository Structure

```
mq2-gas-detector-car/
├── gas_detector_car.ino        Main firmware (single file)
├── README.md
```

---

## Calibration

### MQ-2 Gas Sensor

> Allow **60–120 seconds** of warm-up time after power-on before relying on readings.

| Condition | Typical ADC Value |
|-----------|:-----------------:|
| Clean ambient air | 2,600 – 2,700 |
| Lighter gas near sensor | ~4,095 |
| **Configured alarm threshold** | **≥ 3,200** |

To recalibrate for a different gas type or environment:

1. Power on the car and open the Serial Monitor.
2. Record the stable baseline reading in clean air.
3. Briefly introduce your target gas and note the peak ADC value.
4. Set `GAS_THRESHOLD` to approximately the midpoint between the two.
5. Re-upload the firmware.

### HC-SR04 Ultrasonic Sensors

The 20 cm obstacle threshold balances early detection against unnecessary avoidance manoeuvres. Increase this value in tighter spaces; decrease it for high-speed operation. Readings of `999` indicate no echo was received within the 30 ms `pulseIn()` timeout — the path is clear.

---

## Test Results

All 9 test cases passed during final system validation on 18/12/2025.

| ID | Scenario | Expected Behaviour | Result |
|----|----------|--------------------|:------:|
| TC01 | Normal air, no gas | Buzzer OFF · car drives forward | ✅ Pass |
| TC02 | Gas source near MQ-2 | Motors stop · buzzer ON | ✅ Pass |
| TC03 | Gas source removed | Buzzer OFF · car resumes roaming | ✅ Pass |
| TC04 | Obstacle at front-left (< 20 cm) | Car turns right | ✅ Pass |
| TC05 | Obstacle at front-right (< 20 cm) | Car turns left | ✅ Pass |
| TC06 | Both front sensors blocked | Car reverses (rear is clear) | ✅ Pass |
| TC07 | Front and rear all blocked | Car rotates in place | ✅ Pass |
| TC08 | Rear obstacle only, front clear | Car continues forward | ✅ Pass |
| TC09 | Gas detected mid-avoidance manoeuvre | Motors stop · buzzer ON (gas overrides movement) | ✅ Pass |

### Objective Validation

| Objective | Outcome |
|-----------|---------|
| Detect hazardous gas during roaming | ✅ Achieved — MQ-2 reads every loop cycle; threshold triggered reliably |
| Immediate alert and motor stop | ✅ Achieved — response measured at < 1 s from threshold crossing |
| Autonomous obstacle avoidance | ✅ Achieved — all four quadrants covered; no deadlock observed in testing |
| Real-time Serial Monitor debugging | ✅ Achieved — gas value and all four distances printed every cycle |

---

## Project Budget

**Total cost: RM 197.44** — entirely self-funded by the project team.

| # | Item | Qty | Unit (RM) | Total (RM) |
|---|------|:---:|:---------:|:----------:|
| 1 | ESP32 DevKitC V4 (38-pin + extension board) | 1 | 25.50 | 25.50 |
| 2 | Robot Wheel Base 2WD (chassis + L298N + motors + wheels) | 1 | 28.00 | 28.00 |
| 3 | Full-size Breadboard | 1 | 3.00 | 3.00 |
| 4 | 18650 Li-ion Battery 2200 mAh | 2 | 7.14 | 14.28 |
| 5 | 18650 Battery Holder (2-slot) | 2 | 1.33 | 2.66 |
| 6 | 18650 Battery Charger | 1 | 12.27 | 12.27 |
| 7 | Electronics Beginner Component Kit | 1 | 31.63 | 31.63 |
| 8 | MQ-2 Gas Sensor | 1 | 12.00 | 12.00 |
| 9 | Dupont Jumper Wires 15 cm (40 pcs) | 1 | 7.90 | 7.90 |
| 10 | 3D-Printed Ultrasonic Bracket | 4 | 3.50 | 14.00 |
| 11 | HC-SR04 Ultrasonic Sensor | 5 | 6.00 | 30.00 |
| 12 | Buzzer 3–24 V DC | 1 | 2.00 | 2.00 |
| 13 | Lighter (gas simulation source) | 1 | 0.70 | 0.70 |
| 14 | Storage Box 10 L | 1 | 13.50 | 13.50 |
| | | | **Total** | **RM 197.44** |

---

## Known Limitations & Future Work

- **MQ-2 airflow sensitivity** — the robot's movement can cause slight ADC fluctuations. Multi-sample averaging would reduce false positives in high-mobility scenarios.
- **Two-wheel drive** — a 4WD platform would improve manoeuvrability in tight corners and on uneven surfaces.
- **No wireless alerting** — the ESP32's onboard Wi-Fi and Bluetooth are unused. Future iterations could push gas detection events to a mobile app or cloud dashboard via MQTT.
- **Reactive navigation** — the robot does not maintain a map or coverage path. A lawn-mower or wall-following algorithm would guarantee systematic area coverage.

---

## Team

| Name | Student ID |
|------|:----------:|
| Chan Boon Hong | 52213123434 |
| Akmal Hakimi Bin Abd Rashid | 52213123466 |
| Ti Hui Xian | 52215123002 |

**Lecturer:** Ts. Siti Fatimah Omar  
**Course:** ISB37503 — Realtime and Embedded System  
**Submission date:** 18 December 2025  
**Institution:** Universiti Kuala Lumpur (UniKL)

---

## License

This project was developed for academic purposes at Universiti Kuala Lumpur (UniKL). You are free to use, adapt, and build upon this work with appropriate attribution to the original authors.

---

<div align="center">
<sub>Built at UniKL · Making enclosed spaces safer through autonomous real-time sensing</sub>
</div>
