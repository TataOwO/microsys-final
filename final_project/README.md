# Machine Operation Control System

## Final Project - Microprocessor Systems Lab (Fall 2025)

A Qt-based machine operation control system that simulates industrial equipment with 4 operational items, safety monitoring via proximity sensor (photoresistor), and a state machine controlling execution flow.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [System Backstory](#system-backstory)
3. [Hardware Requirements](#hardware-requirements)
4. [Wiring Diagram](#wiring-diagram)
5. [Software Requirements](#software-requirements)
6. [Project Setup](#project-setup)
7. [Building the Project](#building-the-project)
8. [Running the Application](#running-the-application)
9. [Running Tests](#running-tests)
10. [User Guide](#user-guide)
11. [Troubleshooting](#troubleshooting)

---

## Project Overview

This project fulfills the basic requirements (60%) of the final project:

| Item | Requirement | Implementation |
|------|-------------|----------------|
| **Item 1 (20%)** | GUI Interface + 4 shortcuts + data input | Qt GUI with 6+ keyboard shortcuts and threshold input |
| **Item 2 (20%)** | GPIO control of 4 LEDs (blink + steady) | 4 LEDs with state-based control (steady, blink, alarm) |
| **Item 3 (20%)** | Photoresistor + MCP3008 + LED control | ADC reading as proximity sensor triggering safety alarm |

### Features

- **State Machine Architecture**: STANDBY → EXECUTING → ALARM states
- **Safety Monitoring**: Proximity sensor (photoresistor) triggers alarm when object detected
- **Visual Feedback**: LEDs indicate system state (steady = ready, blink = executing, fast blink = alarm)
- **Lock Mechanism**: System locks during execution and alarm states
- **Configurable Threshold**: Adjustable safety sensor sensitivity

---

## System Backstory

This system simulates a **Machine Operation Control System** used in industrial settings:

### Scenario
- An operator controls a machine with **4 operational items** (represented by 4 LEDs)
- A **proximity sensor** monitors if anyone approaches the machine during operation
- For safety, if the sensor detects proximity (object blocking light), the system triggers an **ALARM**
- The operator must manually dismiss the alarm before resuming operations

### State Machine

```
                    ┌──────────────────────────────────────┐
                    │                                      │
                    ▼                                      │
              ┌──────────┐         Press 1-4          ┌─────────────┐
              │  STANDBY │────────────────────────────│ EXECUTING_N │
              │          │                            │             │
              │ All LEDs │         Completed          │ LED N blinks│
              │ ON steady│◄───────────────────────────│ Others ON   │
              └────┬─────┘                            └──────┬──────┘
                   │                                         │
                   │  ADC > Threshold                        │ ADC > Threshold
                   │  (Proximity detected)                   │ (Proximity detected)
                   │                                         │
                   ▼                                         ▼
              ┌──────────────────────────────────────────────────┐
              │                    ALARM                          │
              │                                                   │
              │              All LEDs blink FAST                  │
              │              System LOCKED                        │
              │                                                   │
              │              Press U to dismiss                   │
              └───────────────────────────────────────────────────┘
```

---

## Hardware Requirements

### Components Needed

| Component | Quantity | Description |
|-----------|----------|-------------|
| NVIDIA Jetson TX2 | 1 | Development board |
| LED | 4 | Any color (red recommended for visibility) |
| Resistor | 4 | 220Ω - 330Ω (for LEDs) |
| Photoresistor (LDR) | 1 | Light dependent resistor (as proximity sensor) |
| Resistor | 1 | 10kΩ (for photoresistor voltage divider) |
| MCP3008 | 1 | 8-channel 10-bit ADC |
| Breadboard | 1 | For circuit assembly |
| Jumper wires | ~20 | Male-to-male and male-to-female |

---

## Wiring Diagram

### GPIO Pin Connections (LEDs)

```
TX2 GPIO Pin    Physical Pin    Component
───────────────────────────────────────────
GPIO 396        Pin 7  (P7)     LED 1 (Item 1)
GPIO 397        Pin 13 (P13)    LED 2 (Item 2)
GPIO 255        Pin 15 (P15)    LED 3 (Item 3)
GPIO 297        Pin 32 (P32)    LED 4 (Item 4)
GND             Pin 6           All LED (-) via 220Ω resistors
```

### LED Wiring Detail
```
GPIO Pin ──────┬──── LED (+) ──── LED (-) ──── 220Ω ──── GND
               │
         (Pin 7, 13, 15, or 32)
```

### MCP3008 ADC Connections (SPI)

```
MCP3008 Pin     TX2 Physical Pin    Description
─────────────────────────────────────────────────
VDD (16)        Pin 1  (3.3V)       Power supply
VREF (15)       Pin 1  (3.3V)       Reference voltage
AGND (14)       Pin 6  (GND)        Analog ground
CLK (13)        Pin 23 (SPICLK)     SPI Clock
DOUT (12)       Pin 21 (SPIMISO)    SPI Data Out (MISO)
DIN (11)        Pin 19 (SPIMOSI)    SPI Data In (MOSI)
CS (10)         Pin 24 (SPICS)      Chip Select
DGND (9)        Pin 6  (GND)        Digital ground
CH0 (1)         Photoresistor       Analog input channel 0
```

### Photoresistor Circuit (Proximity Sensor)
```
3.3V ──── Photoresistor ────┬──── MCP3008 CH0 (Pin 1)
                            │
                          10kΩ
                            │
                           GND

Note: HIGH ADC value = Dark = Something blocking sensor = DANGER
      LOW ADC value = Bright = Clear path = SAFE
```

---

## Software Requirements

### On the TX2

- Ubuntu 18.04 (L4T)
- Qt 5.x development libraries
- Python 3.x
- Jetson.GPIO library

### Install Dependencies (if not already installed)

```bash
# Qt development tools
sudo apt-get update
sudo apt-get install qt5-default qtcreator

# Python GPIO library
sudo pip3 install Jetson.GPIO

# Build tools
sudo apt-get install build-essential
```

---

## Project Setup

### Step 1: Clone or Copy the Project

```bash
cd /home/user
git clone <repository-url> microsys-final
```

### Step 2: Navigate to Project Directory

```bash
cd /home/user/microsys-final/final_project
```

### Step 3: Verify All Files Are Present

```bash
ls -la
```

---

## Building the Project

### Command Line Build

```bash
cd /home/user/microsys-final/final_project

# Generate Makefile
qmake final_project.pro

# Compile
make

# The executable will be created as 'final_project'
```

---

## Running the Application

### Step 1: Connect Hardware

1. Wire the LEDs to GPIO pins (P7, P13, P15, P32)
2. Wire the MCP3008 ADC to SPI pins
3. Connect the photoresistor to MCP3008 CH0
4. Double-check all connections

### Step 2: Run the Application

```bash
cd /home/user/microsys-final/final_project

# Run with sudo (required for GPIO access)
sudo ./final_project
```

### Step 3: Using the Application

1. Click **"Start Monitor [M]"** to enable safety monitoring
2. Press **1-4** to execute machine operations
3. Watch the LEDs blink during execution
4. If something blocks the proximity sensor, the alarm triggers
5. Press **U** to dismiss the alarm

---

## Running Tests

### Quick Hardware Test

```bash
cd /home/user/microsys-final/final_project/tests
make
sudo ./run_all_tests.sh --quick
```

---

## User Guide

### GUI Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│             Machine Operation Control System                         │
├─────────────────────────────────────────────────────────────────────┤
│  State: STANDBY    Lock: UNLOCKED    Monitor: OFF    Executing: None│
├────────────────────────────────────┬────────────────────────────────┤
│   Machine Items (LEDs)             │   Execution Progress           │
│  ┌────┐  ┌────┐  ┌────┐  ┌────┐   │  ┌──────────────────────────┐  │
│  │LED1│  │LED2│  │LED3│  │LED4│   │  │████████████░░░░░░░░░░░░░│  │
│  └────┘  └────┘  └────┘  └────┘   │  └──────────────────────────┘  │
│   [Execute 1] [Execute 2] ...      │   Ready for operation          │
├────────────────────────────────────┼────────────────────────────────┤
│   Safety Monitoring                │   Safety Threshold             │
│   ADC: [████████░░░░] 350         │   Threshold: [__500__] [Set]   │
│   Safety: SAFE                     │   Current: 500                 │
│   [Start Monitor] [Stop] [Dismiss] │   ADC > Threshold = DANGER     │
├─────────────────────────────────────────────────────────────────────┤
│  [1-4] Execute    [U] Unlock    [M] Monitor    [ESC] Stop    [H] Help│
└─────────────────────────────────────────────────────────────────────┘
```

### Keyboard Shortcuts

| Key | Function |
|-----|----------|
| `1` | Execute Item 1 (LED1 blinks) |
| `2` | Execute Item 2 (LED2 blinks) |
| `3` | Execute Item 3 (LED3 blinks) |
| `4` | Execute Item 4 (LED4 blinks) |
| `U` | Unlock / Dismiss Alarm |
| `M` | Toggle Safety Monitoring |
| `ESC` | Emergency Stop |
| `H` | Show help dialog |

### System States

**STANDBY (Ready)**
- All 4 LEDs are ON steadily
- System is UNLOCKED
- Press 1-4 to start an operation

**EXECUTING (Working)**
- Selected item's LED blinks (300ms interval)
- Other LEDs remain ON
- System is LOCKED
- Progress bar shows completion percentage
- Duration: 5 seconds per operation

**ALARM (Danger)**
- ALL LEDs blink rapidly (100ms interval)
- System is LOCKED
- Triggered when ADC > threshold (object detected)
- Press U to dismiss and return to STANDBY

### Safety Monitoring

The photoresistor acts as a proximity sensor:
- **LOW ADC** (bright, clear path) → **SAFE** (green)
- **HIGH ADC** (dark, blocked) → **DANGER** (red) → **ALARM**

Adjust the threshold to calibrate sensitivity:
- Lower threshold = More sensitive (triggers alarm easier)
- Higher threshold = Less sensitive

---

## Troubleshooting

### Common Issues

#### 1. "Permission denied" when running

```bash
sudo ./final_project
```

#### 2. LEDs not lighting up

- Check wiring connections
- Verify LED polarity (longer leg = positive)
- Test GPIO: `sudo ./tests/test_gpio 3`

#### 3. ADC always returns 0

- Verify MCP3008 wiring (VDD, GND, SPI pins)
- Test ADC: `sudo python3 tests/test_adc.py 1`

#### 4. Alarm triggers too easily/rarely

- Adjust the safety threshold (0-1023)
- Higher value = less sensitive
- Lower value = more sensitive

#### 5. System won't execute items

- Make sure system is in STANDBY state
- Dismiss any active alarms (press U)
- Check if system is locked (wait for current operation)

---

## Project Structure

```
final_project/
├── main.cpp              # Application entry point
├── mainwindow.h          # Main window header (state machine)
├── mainwindow.cpp        # Main window implementation
├── mainwindow.ui         # Qt Designer UI file
├── final_project.pro     # Qt project file
├── gpio_util.hpp/cpp     # GPIO control utilities
├── gpio_LED_CTRL.hpp/cpp # LED controller class
├── PIN_LOOKUP.hpp        # GPIO pin mappings
├── read_adc.py           # MCP3008 ADC reader (Python)
├── qtres.qrc             # Qt resources
├── lightbulb.png         # LED icon
├── README.md             # This file
├── new_scenario_plan.md  # State machine design document
└── tests/
    ├── Makefile
    ├── test_gpio.cpp
    ├── test_led.cpp
    ├── test_adc.py
    └── run_all_tests.sh
```

---

## Quick Start Summary

```bash
# 1. Navigate to project
cd /home/user/microsys-final/final_project

# 2. Build
qmake && make

# 3. Run application
sudo ./final_project

# 4. In the app:
#    - Click "Start Monitor" or press M
#    - Press 1-4 to execute items
#    - Watch LEDs blink during execution
#    - Block the photoresistor to trigger alarm
#    - Press U to dismiss alarm
```

---

*Machine Operation Control System - Microprocessor Systems Lab Final Project*
