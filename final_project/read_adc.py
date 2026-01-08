import Jetson.GPIO as GPIO
import time
import sys

# SPI pins (BOARD numbering - Physical pin numbers)
SPICLK = 23   # Board pin 23 (BCM 11)
SPIMISO = 21  # Board pin 21 (BCM 9)
SPIMOSI = 19  # Board pin 19 (BCM 10)
SPICS = 24    # Board pin 24 (BCM 8)

# photoresistor connected to adc #0
photo_ch = 0

# read SPI data from MCP3008 chip, 8 possible adc's (0 thru 7)
def readadc(adcnum, clockpin, mosipin, misopin, cspin):
    if ((adcnum > 7) or (adcnum < 0)):
        return -1
    GPIO.output(cspin, True)

    GPIO.output(clockpin, False)  # start clock low
    GPIO.output(cspin, False)     # bring CS low

    commandout = adcnum
    commandout |= 0x18  # start bit + single-ended bit
    commandout <<= 3    # we only need to send 5 bits here
    
    for i in range(5):
        if (commandout & 0x80):
            GPIO.output(mosipin, True)
        else:
            GPIO.output(mosipin, False)
        commandout <<= 1
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)
    
    adcout = 0
    # read in one empty bit, one null bit and 10 ADC bits
    for i in range(12):
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)
        adcout <<= 1
        if (GPIO.input(misopin)):
            adcout |= 0x1

    GPIO.output(cspin, True)
    adcout >>= 1  # first bit is 'null' so drop it
    return adcout

def main():
    try:
        # Use BOARD mode (physical pin numbers)
        GPIO.setmode(GPIO.BOARD)
        GPIO.setwarnings(False)
        
        # Setup SPI pins
        GPIO.setup(SPIMOSI, GPIO.OUT)
        GPIO.setup(SPIMISO, GPIO.IN)
        GPIO.setup(SPICLK, GPIO.OUT)
        GPIO.setup(SPICS, GPIO.OUT)

        # Read ADC value once
        value = readadc(photo_ch, SPICLK, SPIMOSI, SPIMISO, SPICS)
        print(value)  # Print value for C++ to read
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        print(0)  # Return 0 on error
    finally:
        GPIO.cleanup()

if __name__ == '__main__':
    main()
