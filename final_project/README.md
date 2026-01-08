# Smart Light Control System

## Final Project - Microprocessor Systems Lab (Fall 2025)

A Qt-based smart light control system that monitors ambient light using a photoresistor and automatically controls LEDs based on brightness levels.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Wiring Diagram](#wiring-diagram)
4. [Software Requirements](#software-requirements)
5. [Project Setup](#project-setup)
6. [Building the Project](#building-the-project)
7. [Running the Application](#running-the-application)
8. [Running Tests](#running-tests)
9. [User Guide](#user-guide)
10. [Troubleshooting](#troubleshooting)

---

## Project Overview

This project fulfills the basic requirements (60%) of the final project:

| Item | Requirement | Implementation |
|------|-------------|----------------|
| **Item 1 (20%)** | GUI Interface + 4 shortcuts + data input | Qt GUI with 8 keyboard shortcuts and threshold input |
| **Item 2 (20%)** | GPIO control of 4 LEDs (blink + steady) | 4 LEDs with individual/group control and blink patterns |
| **Item 3 (20%)** | Photoresistor + MCP3008 + LED control | ADC reading with auto LED control based on light level |

### Features

- Real-time light level monitoring via MCP3008 ADC
- Automatic LED control based on configurable threshold
- Manual LED control with keyboard shortcuts
- Visual feedback with progress bars and status indicators
- Multiple LED patterns (blink, chase, etc.)

---

## Hardware Requirements

### Components Needed

| Component | Quantity | Description |
|-----------|----------|-------------|
| NVIDIA Jetson TX2 | 1 | Development board |
| LED | 4 | Any color (red recommended for visibility) |
| Resistor | 4 | 220Ω - 330Ω (for LEDs) |
| Photoresistor (LDR) | 1 | Light dependent resistor |
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
GPIO 396        Pin 7  (P7)     LED 1 (+)
GPIO 397        Pin 13 (P13)    LED 2 (+)
GPIO 255        Pin 15 (P15)    LED 3 (+)
GPIO 297        Pin 32 (P32)    LED 4 (+)
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

### Photoresistor Circuit
```
3.3V ──── Photoresistor ────┬──── MCP3008 CH0 (Pin 1)
                            │
                          10kΩ
                            │
                           GND
```

### Complete Wiring Overview
```
                    ┌─────────────────────────────────────┐
                    │           JETSON TX2                │
                    │                                     │
    3.3V ─────────── Pin 1                                │
    GND ──────────── Pin 6                                │
                    │                                     │
    LED1 (+) ─────── Pin 7  (GPIO 396)                    │
    LED2 (+) ─────── Pin 13 (GPIO 397)                    │
    LED3 (+) ─────── Pin 15 (GPIO 255)                    │
    LED4 (+) ─────── Pin 32 (GPIO 297)                    │
                    │                                     │
    SPIMOSI ──────── Pin 19                               │
    SPIMISO ──────── Pin 21                               │
    SPICLK ───────── Pin 23                               │
    SPICS ────────── Pin 24                               │
                    └─────────────────────────────────────┘

                    ┌─────────────────────────────────────┐
                    │           MCP3008                   │
                    │                                     │
    Photoresistor ── CH0 (Pin 1)                          │
                    │                                     │
    VDD ──────────── Pin 16 ─── 3.3V                      │
    VREF ─────────── Pin 15 ─── 3.3V                      │
    AGND ─────────── Pin 14 ─── GND                       │
    CLK ──────────── Pin 13 ─── TX2 Pin 23                │
    DOUT ─────────── Pin 12 ─── TX2 Pin 21                │
    DIN ──────────── Pin 11 ─── TX2 Pin 19                │
    CS ───────────── Pin 10 ─── TX2 Pin 24                │
    DGND ─────────── Pin 9  ─── GND                       │
                    └─────────────────────────────────────┘
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
# If using git
cd /home/user
git clone <repository-url> microsys-final

# Or copy the project folder to the TX2
```

### Step 2: Navigate to Project Directory

```bash
cd /home/user/microsys-final/final_project
```

### Step 3: Verify All Files Are Present

```bash
ls -la
```

Expected files:
```
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── final_project.pro
├── gpio_util.hpp / .cpp
├── gpio_LED_CTRL.hpp / .cpp
├── PIN_LOOKUP.hpp
├── read_adc.py
├── qtres.qrc
├── lightbulb.png
└── tests/
```

---

## Building the Project

### Option 1: Command Line Build

```bash
cd /home/user/microsys-final/final_project

# Generate Makefile
qmake final_project.pro

# Compile
make

# The executable will be created as 'final_project'
```

### Option 2: Using Qt Creator

1. Open Qt Creator
2. File → Open File or Project
3. Select `final_project.pro`
4. Click "Configure Project"
5. Build → Build Project (Ctrl+B)

### Verify Build Success

```bash
# Check if executable exists
ls -la final_project

# Or run the build verification test
cd tests
./verify_build.sh
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

1. Click **"Start"** to begin monitoring
2. The ADC value will update every 500ms
3. Use keyboard shortcuts or buttons to control LEDs
4. Adjust threshold to change auto-control sensitivity

---

## Running Tests

### Quick Hardware Test

```bash
cd /home/user/microsys-final/final_project/tests

# Build tests
make

# Run all tests
sudo ./run_all_tests.sh
```

### Individual Tests

```bash
# Test GPIO functionality
sudo ./test_gpio 4

# Test LED controller
sudo ./test_led 2

# Test ADC reading
sudo python3 test_adc.py 2

# Interactive LED test
sudo ./test_led 4
```

### Test Options

```bash
# Quick test (skip long stability tests)
sudo ./run_all_tests.sh --quick

# Test only GPIO
sudo ./run_all_tests.sh --gpio-only

# Test only LEDs
sudo ./run_all_tests.sh --led-only

# Test only ADC
sudo ./run_all_tests.sh --adc-only
```

---

## User Guide

### GUI Layout

```
┌─────────────────────────────────────────────────────────┐
│         Smart Light Control System                      │
├─────────────────────────────────────────────────────────┤
│   [LED1]    [LED2]    [LED3]    [LED4]                 │
│    (1)       (2)       (3)       (4)                   │
├─────────────────────────────────────────────────────────┤
│   Light Sensor: [████████░░░░░] 650                    │
│   Status: BRIGHT                                        │
├─────────────────────────────────────────────────────────┤
│   Threshold: [___500___] [Set]    Mode: MANUAL         │
├─────────────────────────────────────────────────────────┤
│   [Start] [Stop] [Blink] [All On] [All Off]            │
└─────────────────────────────────────────────────────────┘
```

### Keyboard Shortcuts

| Key | Function |
|-----|----------|
| `1` | Toggle LED 1 |
| `2` | Toggle LED 2 |
| `3` | Toggle LED 3 |
| `4` | Toggle LED 4 |
| `A` | Toggle Auto/Manual mode |
| `B` | Blink all LEDs |
| `S` | Stop system |
| `R` | Read ADC manually |
| `H` | Show help dialog |

### Operation Modes

**Manual Mode:**
- Control LEDs individually with keys 1-4
- Use buttons for group control
- ADC is monitored but doesn't control LEDs

**Auto Mode:**
- LEDs are controlled automatically based on light level
- If ADC < threshold → LEDs turn ON (dark environment)
- If ADC ≥ threshold → LEDs turn OFF (bright environment)
- Individual LED control is disabled

### Setting Threshold

1. Enter a value (0-1023) in the threshold input field
2. Click "Set" or press Enter
3. The threshold determines when LEDs turn on/off in Auto mode
   - Lower threshold = LEDs turn on in darker conditions
   - Higher threshold = LEDs turn on in brighter conditions

---

## Troubleshooting

### Common Issues

#### 1. "Permission denied" when running

```bash
# Solution: Run with sudo
sudo ./final_project
```

#### 2. LEDs not lighting up

- Check wiring connections
- Verify LED polarity (longer leg = positive)
- Test GPIO with: `sudo ./tests/test_gpio 3`
- Check if resistors are connected

#### 3. ADC always returns 0

- Verify MCP3008 wiring (especially VDD and GND)
- Check SPI pin connections
- Test ADC with: `sudo python3 tests/test_adc.py 1`
- Run diagnostic: `sudo python3 read_adc.py`

#### 4. "gpio/export: Permission denied"

```bash
# Make sure to run with sudo
sudo ./final_project

# Or add user to gpio group (requires reboot)
sudo usermod -a -G gpio $USER
```

#### 5. Qt application doesn't start

```bash
# Check if display is available
echo $DISPLAY

# If empty, set it
export DISPLAY=:0

# Then run
sudo ./final_project
```

#### 6. "Cannot find read_adc.py"

```bash
# Copy script to build directory
cp read_adc.py /path/to/build/directory/

# Or run from project directory
cd /home/user/microsys-final/final_project
sudo ./final_project
```

### LED Pin Quick Reference

| LED | GPIO | Physical Pin | Test Command |
|-----|------|--------------|--------------|
| LED1 | 396 | Pin 7 | `sudo ./test_led 1` |
| LED2 | 397 | Pin 13 | `sudo ./test_led 1` |
| LED3 | 255 | Pin 15 | `sudo ./test_led 1` |
| LED4 | 297 | Pin 32 | `sudo ./test_led 1` |

### Getting Help

Press `H` in the application to see keyboard shortcuts, or run:

```bash
cd tests
make help
```

---

## Project Structure

```
final_project/
├── main.cpp              # Application entry point
├── mainwindow.h          # Main window header
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
└── tests/
    ├── Makefile          # Test build system
    ├── test_gpio.cpp     # GPIO tests
    ├── test_led.cpp      # LED tests
    ├── test_adc.py       # ADC tests
    ├── run_all_tests.sh  # Test runner
    └── verify_build.sh   # Build verification
```

---

## Quick Start Summary

```bash
# 1. Navigate to project
cd /home/user/microsys-final/final_project

# 2. Build
qmake && make

# 3. Run tests (optional)
cd tests && make && sudo ./run_all_tests.sh --quick && cd ..

# 4. Run application
sudo ./final_project

# 5. In the app:
#    - Click "Start" to begin
#    - Press 1-4 to toggle LEDs
#    - Press A to enable auto mode
#    - Adjust threshold as needed
```

---

*Smart Light Control System - Microprocessor Systems Lab Final Project*
