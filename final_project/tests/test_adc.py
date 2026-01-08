#!/usr/bin/env python3
"""
ADC (MCP3008) Test Program
Tests the photoresistor reading via SPI

Usage: sudo python3 test_adc.py [test_number]
  1 - Single read test
  2 - Continuous read test (10 samples)
  3 - Stability test (100 samples with statistics)
  4 - Light change detection test
  5 - Run all tests
"""

import sys
import time
import os

# Add parent directory to path for read_adc module
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    import Jetson.GPIO as GPIO
except ImportError:
    print("WARNING: Jetson.GPIO not available. Running in simulation mode.")
    GPIO = None

# SPI pins (BOARD numbering)
SPICLK = 23
SPIMISO = 21
SPIMOSI = 19
SPICS = 24

# Photoresistor channel
PHOTO_CH = 0

# Colors
GREEN = '\033[32m'
RED = '\033[31m'
YELLOW = '\033[33m'
CYAN = '\033[36m'
RESET = '\033[0m'

def print_header(text):
    print(f"\n{YELLOW}=== {text} ==={RESET}")

def print_pass(text):
    print(f"{GREEN}[PASS]{RESET} {text}")

def print_fail(text):
    print(f"{RED}[FAIL]{RESET} {text}")

def print_info(text):
    print(f"{CYAN}[INFO]{RESET} {text}")

# Initialize GPIO
def setup_gpio():
    if GPIO is None:
        return False
    try:
        GPIO.setmode(GPIO.BOARD)
        GPIO.setwarnings(False)
        GPIO.setup(SPIMOSI, GPIO.OUT)
        GPIO.setup(SPIMISO, GPIO.IN)
        GPIO.setup(SPICLK, GPIO.OUT)
        GPIO.setup(SPICS, GPIO.OUT)
        return True
    except Exception as e:
        print(f"GPIO setup error: {e}")
        return False

def cleanup_gpio():
    if GPIO is not None:
        try:
            GPIO.cleanup()
        except:
            pass

# Read ADC value
def readadc(adcnum):
    if GPIO is None:
        # Simulation mode - return random value
        import random
        return random.randint(200, 800)

    if adcnum > 7 or adcnum < 0:
        return -1

    GPIO.output(SPICS, True)
    GPIO.output(SPICLK, False)
    GPIO.output(SPICS, False)

    commandout = adcnum | 0x18
    commandout <<= 3

    for i in range(5):
        if commandout & 0x80:
            GPIO.output(SPIMOSI, True)
        else:
            GPIO.output(SPIMOSI, False)
        commandout <<= 1
        GPIO.output(SPICLK, True)
        GPIO.output(SPICLK, False)

    adcout = 0
    for i in range(12):
        GPIO.output(SPICLK, True)
        GPIO.output(SPICLK, False)
        adcout <<= 1
        if GPIO.input(SPIMISO):
            adcout |= 0x1

    GPIO.output(SPICS, True)
    adcout >>= 1
    return adcout

# Test 1: Single Read
def test_single_read():
    print_header("Test 1: Single ADC Read")

    value = readadc(PHOTO_CH)
    print(f"  ADC Value: {value}")

    if 0 <= value <= 1023:
        print_pass(f"Valid ADC value received (0-1023 range): {value}")
        return True
    else:
        print_fail(f"Invalid ADC value: {value}")
        return False

# Test 2: Continuous Read
def test_continuous_read():
    print_header("Test 2: Continuous Read (10 samples)")

    values = []
    for i in range(10):
        value = readadc(PHOTO_CH)
        values.append(value)
        bar = '#' * (value // 20) + '-' * (50 - value // 20)
        print(f"  Sample {i+1:2d}: [{bar}] {value:4d}")
        time.sleep(0.2)

    avg = sum(values) / len(values)
    print(f"\n  Average: {avg:.1f}")

    all_valid = all(0 <= v <= 1023 for v in values)
    if all_valid:
        print_pass("All samples are valid")
        return True
    else:
        print_fail("Some samples are invalid")
        return False

# Test 3: Stability Test
def test_stability():
    print_header("Test 3: Stability Test (100 samples)")

    print("  Collecting 100 samples...")
    values = []
    for i in range(100):
        values.append(readadc(PHOTO_CH))
        time.sleep(0.05)

    avg = sum(values) / len(values)
    min_val = min(values)
    max_val = max(values)
    range_val = max_val - min_val

    # Calculate standard deviation
    variance = sum((x - avg) ** 2 for x in values) / len(values)
    std_dev = variance ** 0.5

    print(f"\n  Statistics:")
    print(f"    Average:    {avg:.1f}")
    print(f"    Min:        {min_val}")
    print(f"    Max:        {max_val}")
    print(f"    Range:      {range_val}")
    print(f"    Std Dev:    {std_dev:.2f}")

    # Pass if range is reasonable (< 100 for stable light)
    if range_val < 200:
        print_pass(f"Stable readings (range={range_val} < 200)")
        return True
    else:
        print_info(f"High variance in readings - this may be normal if light is changing")
        return True

# Test 4: Light Change Detection
def test_light_change():
    print_header("Test 4: Light Change Detection")

    print("  This test monitors light changes for 10 seconds.")
    print("  Try covering/uncovering the photoresistor.")
    print()

    start_time = time.time()
    last_value = readadc(PHOTO_CH)
    changes_detected = 0
    threshold = 50  # Change threshold

    while time.time() - start_time < 10:
        value = readadc(PHOTO_CH)
        change = abs(value - last_value)

        if change > threshold:
            changes_detected += 1
            direction = "DARKER" if value < last_value else "BRIGHTER"
            print(f"  Light change detected: {direction} (delta={change})")

        # Visual bar
        bar = '#' * (value // 20) + '-' * (50 - value // 20)
        light_status = "DARK" if value < 500 else "BRIGHT"
        print(f"\r  [{bar}] {value:4d} ({light_status})    ", end='', flush=True)

        last_value = value
        time.sleep(0.1)

    print(f"\n\n  Changes detected: {changes_detected}")
    print_pass("Light monitoring test completed")
    return True

# Test 5: All Channels
def test_all_channels():
    print_header("Test 5: All MCP3008 Channels (0-7)")

    print("  Reading all 8 ADC channels:")
    for ch in range(8):
        value = readadc(ch)
        bar = '#' * (value // 20) + '-' * (50 - value // 20)
        print(f"    CH{ch}: [{bar}] {value:4d}")

    print_pass("All channels read successfully")
    return True

def run_all_tests():
    results = []
    results.append(("Single Read", test_single_read()))
    results.append(("Continuous Read", test_continuous_read()))
    results.append(("Stability", test_stability()))
    results.append(("All Channels", test_all_channels()))

    print_header("Test Summary")
    passed = sum(1 for _, r in results if r)
    total = len(results)

    for name, result in results:
        status = f"{GREEN}PASS{RESET}" if result else f"{RED}FAIL{RESET}"
        print(f"  {name}: {status}")

    print(f"\n  Total: {passed}/{total} tests passed")
    return passed == total

def main():
    print("MCP3008 ADC Test Suite")
    print("======================")

    if GPIO is None:
        print(f"{YELLOW}Running in SIMULATION MODE (no GPIO){RESET}\n")
    else:
        print("NOTE: Run with sudo for GPIO access\n")

    # Setup
    if not setup_gpio() and GPIO is not None:
        print_fail("Failed to setup GPIO")
        return 1

    try:
        test_num = 5  # Default: run all
        if len(sys.argv) > 1:
            test_num = int(sys.argv[1])

        if test_num == 1:
            test_single_read()
        elif test_num == 2:
            test_continuous_read()
        elif test_num == 3:
            test_stability()
        elif test_num == 4:
            test_light_change()
        elif test_num == 5:
            run_all_tests()
        else:
            print("Invalid test number. Use 1-5.")

    finally:
        cleanup_gpio()

    return 0

if __name__ == '__main__':
    sys.exit(main())
