# Quick Start Guide - Machine Operation Control System

## 1. Hardware Setup (5 minutes)

### LED Connections (4 Machine Items)
```
LED 1 (Item 1) → Pin 7  (GPIO 396) → 220Ω → GND
LED 2 (Item 2) → Pin 13 (GPIO 397) → 220Ω → GND
LED 3 (Item 3) → Pin 15 (GPIO 255) → 220Ω → GND
LED 4 (Item 4) → Pin 32 (GPIO 297) → 220Ω → GND
```

### MCP3008 Connections (Proximity Sensor)
```
MCP3008        TX2
────────────────────
VDD, VREF  →   3.3V (Pin 1)
AGND, DGND →   GND  (Pin 6)
CLK        →   Pin 23
DOUT       →   Pin 21
DIN        →   Pin 19
CS         →   Pin 24
CH0        →   Photoresistor
```

### Photoresistor (Proximity Sensor)
```
3.3V → Photoresistor → MCP3008 CH0
                    ↓
                  10kΩ → GND
```

---

## 2. Build & Run (2 minutes)

```bash
cd /home/user/microsys-final/final_project
qmake && make
sudo ./final_project
```

---

## 3. Test Hardware (Optional)

```bash
cd tests
make
sudo ./run_all_tests.sh --quick
```

---

## 4. Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **1-4** | Execute Item 1-4 (LED blinks) |
| **U** | Unlock / Dismiss Alarm |
| **M** | Toggle Safety Monitoring |
| **ESC** | Emergency Stop |
| **H** | Help |

---

## 5. System States

| State | LEDs | Lock Status |
|-------|------|-------------|
| **STANDBY** | All ON steady | Unlocked |
| **EXECUTING** | Selected blinks, others ON | Locked |
| **ALARM** | All blink fast | Locked |

---

## 6. Demo Checklist

- [ ] Hardware wired correctly
- [ ] Run `sudo ./final_project`
- [ ] Click **Start Monitor** (or press M)
- [ ] Press **1** to execute Item 1 → LED1 blinks
- [ ] Wait for completion → returns to STANDBY
- [ ] Block photoresistor → **ALARM** triggers
- [ ] Press **U** to dismiss alarm
- [ ] Adjust threshold if needed

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Permission denied | Use `sudo` |
| LEDs not working | Check wiring, run `sudo ./tests/test_gpio 3` |
| ADC reads 0 | Check MCP3008 wiring, run `sudo python3 tests/test_adc.py 1` |
| Alarm too sensitive | Increase threshold value |
| Can't execute items | Dismiss alarm first (press U) |
