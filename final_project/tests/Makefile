# Test Suite Makefile
# Build and run tests for the Smart Light Control System

CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS =

# Source directory
SRC_DIR = ..

# Test programs
TESTS = test_gpio test_led

# Source files from parent directory
GPIO_UTIL = $(SRC_DIR)/gpio_util.cpp
LED_CTRL = $(SRC_DIR)/gpio_LED_CTRL.cpp

.PHONY: all clean test test_gpio_run test_led_run test_adc_run help

all: $(TESTS)
	@echo "Build complete. Run tests with: make test"

# Build test_gpio
test_gpio: test_gpio.cpp $(GPIO_UTIL)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)
	@echo "Built test_gpio"

# Build test_led
test_led: test_led.cpp $(LED_CTRL) $(GPIO_UTIL)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)
	@echo "Built test_led"

# Run tests
test: all
	@echo ""
	@echo "================================"
	@echo "Running all tests..."
	@echo "================================"
	@echo ""
	@echo ">>> GPIO Tests"
	@sudo ./test_gpio 4 || true
	@echo ""
	@echo ">>> LED Tests (automated)"
	@sudo ./test_led 5 || true
	@echo ""
	@echo ">>> ADC Tests"
	@sudo python3 test_adc.py 5 || true
	@echo ""
	@echo "================================"
	@echo "All tests completed!"
	@echo "================================"

# Individual test runners
test_gpio_run: test_gpio
	@echo "Running GPIO tests..."
	@sudo ./test_gpio

test_led_run: test_led
	@echo "Running LED tests..."
	@sudo ./test_led

test_adc_run:
	@echo "Running ADC tests..."
	@sudo python3 test_adc.py

# Interactive tests
interactive: test_led
	@echo "Starting interactive LED test..."
	@sudo ./test_led 4

# Clean
clean:
	rm -f $(TESTS)
	rm -f *.o
	@echo "Cleaned build files"

# Help
help:
	@echo "Test Suite Makefile"
	@echo "==================="
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build all test programs"
	@echo "  test           - Run all tests (requires sudo)"
	@echo "  test_gpio_run  - Run GPIO tests only"
	@echo "  test_led_run   - Run LED tests only"
	@echo "  test_adc_run   - Run ADC tests only"
	@echo "  interactive    - Run interactive LED test"
	@echo "  clean          - Remove built files"
	@echo "  help           - Show this help"
	@echo ""
	@echo "Individual test numbers:"
	@echo "  test_gpio:"
	@echo "    1 - Export/unexport test"
	@echo "    2 - Direction test"
	@echo "    3 - Value test (LED blink)"
	@echo "    4 - All project GPIOs"
	@echo ""
	@echo "  test_led:"
	@echo "    1 - Single LED test"
	@echo "    2 - All 4 LEDs test"
	@echo "    3 - LED patterns"
	@echo "    4 - Interactive mode"
	@echo "    5 - All (except interactive)"
	@echo ""
	@echo "  test_adc.py:"
	@echo "    1 - Single read"
	@echo "    2 - Continuous read"
	@echo "    3 - Stability test"
	@echo "    4 - Light change detection"
	@echo "    5 - All tests"
