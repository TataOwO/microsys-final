#!/usr/bin/env python3
import Jetson.GPIO as GPIO
import time

# SPI pins (BOARD numbering)
SPICLK = 23
SPIMISO = 21
SPIMOSI = 19
SPICS = 24

print("="*60)
print("MCP3008 完整診斷工具")
print("="*60)

# Step 1: Pin State Test
print("\n【步驟 1】檢查 GPIO 接腳狀態")
print("-"*60)

GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

# Setup all pins as inputs first to check initial state
GPIO.setup(SPIMISO, GPIO.IN)
print(f"MISO (Pin {SPIMISO}) 初始狀態: {'HIGH' if GPIO.input(SPIMISO) else 'LOW'}")

# Setup output pins
GPIO.setup(SPIMOSI, GPIO.OUT)
GPIO.setup(SPICLK, GPIO.OUT)
GPIO.setup(SPICS, GPIO.OUT)

# Test output pins
GPIO.output(SPICS, True)
GPIO.output(SPICLK, False)
GPIO.output(SPIMOSI, False)

print(f"✓ 輸出腳位已初始化")
print(f"  CS: HIGH, CLK: LOW, MOSI: LOW")

# Step 2: MISO Sampling Test
print("\n【步驟 2】MISO 訊號採樣測試")
print("-"*60)
print("採樣 100 次 MISO 狀態...")

high_count = 0
low_count = 0

for i in range(100):
    if GPIO.input(SPIMISO):
        high_count += 1
    else:
        low_count += 1
    time.sleep(0.001)

print(f"HIGH 次數: {high_count}")
print(f"LOW 次數: {low_count}")

if high_count == 100:
    print("❌ 警告: MISO 一直是 HIGH - 可能接線錯誤或 MCP3008 無回應")
elif low_count == 100:
    print("❌ 警告: MISO 一直是 LOW - 可能接線錯誤")
else:
    print("✓ MISO 有變化 - 訊號正常")

# Step 3: Basic SPI Communication Test
print("\n【步驟 3】基本 SPI 通訊測試")
print("-"*60)

def test_spi_transaction():
    """執行一次完整的 SPI 交易並監控 MISO"""
    GPIO.output(SPICS, False)  # CS low
    time.sleep(0.0001)
    
    # Send some clock pulses and check MISO
    miso_bits = []
    for i in range(24):
        GPIO.output(SPICLK, True)
        bit = GPIO.input(SPIMISO)
        miso_bits.append('1' if bit else '0')
        GPIO.output(SPICLK, False)
    
    GPIO.output(SPICS, True)  # CS high
    
    return ''.join(miso_bits)

result = test_spi_transaction()
print(f"MISO 原始位元串: {result}")

if result == '1' * 24:
    print("❌ 所有位元都是 1 - MCP3008 可能沒有連接或故障")
elif result == '0' * 24:
    print("❌ 所有位元都是 0 - 可能接線問題")
else:
    print("✓ 有位元變化 - 通訊可能正常")

# Step 4: MCP3008 Reading Test
print("\n【步驟 4】MCP3008 讀取測試")
print("-"*60)

def readadc(adcnum):
    if adcnum > 7 or adcnum < 0:
        return -1
    
    GPIO.output(SPICS, True)
    GPIO.output(SPICLK, False)
    GPIO.output(SPICS, False)
    
    commandout = adcnum
    commandout |= 0x18
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

print("讀取所有 8 個通道:")
for ch in range(8):
    val = readadc(ch)
    binary = bin(val)[2:].zfill(10)
    print(f"  通道 {ch}: {val:4d} (0b{binary})")

# Step 5: Continuous Reading
print("\n【步驟 5】連續讀取測試 (通道 0)")
print("-"*60)
print("請嘗試遮住/照亮光敏電阻...")
print("讀取 20 次，每次間隔 0.5 秒:\n")

last_val = None
changes = 0

for i in range(20):
    val = readadc(0)
    
    if last_val is not None and val != last_val:
        changes += 1
        indicator = " ← 變化!"
    else:
        indicator = ""
    
    bar_len = int(val / 1023 * 40)
    bar = "█" * bar_len + "░" * (40 - bar_len)
    
    print(f"{i+1:2d}. ADC: {val:4d} [{bar}]{indicator}")
    
    last_val = val
    time.sleep(0.5)

print(f"\n總共偵測到 {changes} 次數值變化")

if changes == 0:
    print("\n❌ 診斷結果: 數值從未改變 - 硬體問題")
    print("\n可能原因:")
    print("1. MCP3008 電源未連接 (VDD/VREF 需接 3.3V)")
    print("2. MCP3008 接地未連接 (AGND/DGND 需接 GND)")
    print("3. SPI 接線錯誤")
    print("4. MCP3008 故障")
    print("5. 光敏電阻電路未連接到 CH0")
elif changes < 3:
    print("\n⚠️  診斷結果: 數值幾乎不變 - 可能感測器問題")
    print("\n請檢查:")
    print("1. 光敏電阻電路是否正確連接")
    print("2. 分壓電阻值是否正確 (建議 10kΩ)")
else:
    print("\n✓ 診斷結果: 感測器運作正常!")

# Cleanup
GPIO.cleanup()

print("\n" + "="*60)
print("診斷完成")
print("="*60)
