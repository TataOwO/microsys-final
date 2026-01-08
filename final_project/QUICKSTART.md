# Quick Start Guide

## 1. Hardware Setup (5 minutes)

### LED Connections
```
LED 1 → Pin 7  (GPIO 396) → 220Ω → GND
LED 2 → Pin 13 (GPIO 397) → 220Ω → GND
LED 3 → Pin 15 (GPIO 255) → 220Ω → GND
LED 4 → Pin 32 (GPIO 297) → 220Ω → GND
```

### MCP3008 Connections
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

### Photoresistor
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
| **1-4** | Toggle LED 1-4 |
| **A** | Toggle Auto Mode |
| **B** | Blink All |
| **S** | Stop |
| **H** | Help |

---

## 5. Demo Checklist

- [ ] Hardware wired correctly
- [ ] Run `sudo ./final_project`
- [ ] Click **Start**
- [ ] Show LED toggle (press 1-4)
- [ ] Show threshold input (change value, click Set)
- [ ] Enable **Auto Mode** (press A)
- [ ] Cover photoresistor → LEDs ON
- [ ] Uncover photoresistor → LEDs OFF
- [ ] Show **Blink All** (press B)

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Permission denied | Use `sudo` |
| LEDs not working | Check wiring, run `sudo ./tests/test_gpio 3` |
| ADC reads 0 | Check MCP3008 wiring, run `sudo python3 tests/test_adc.py 1` |
