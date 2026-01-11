#!/bin/bash
#
# Run All Tests for Smart Light Control System
# Usage: sudo ./run_all_tests.sh [options]
#
# Options:
#   --gpio-only     Run only GPIO tests
#   --led-only      Run only LED tests
#   --adc-only      Run only ADC tests
#   --quick         Run quick tests only (skip stability tests)
#   --interactive   Include interactive tests
#   --help          Show help

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${CYAN}"
echo "╔════════════════════════════════════════════════════════════╗"
echo "║     Smart Light Control System - Test Suite                ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check for root
if [ "$EUID" -ne 0 ]; then
    echo -e "${YELLOW}Warning: Not running as root. Some tests may fail.${NC}"
    echo "Consider running: sudo $0"
    echo ""
fi

# Parse arguments
RUN_GPIO=true
RUN_LED=true
RUN_ADC=true
QUICK_MODE=false
INTERACTIVE=false

for arg in "$@"; do
    case $arg in
        --gpio-only)
            RUN_LED=false
            RUN_ADC=false
            ;;
        --led-only)
            RUN_GPIO=false
            RUN_ADC=false
            ;;
        --adc-only)
            RUN_GPIO=false
            RUN_LED=false
            ;;
        --quick)
            QUICK_MODE=true
            ;;
        --interactive)
            INTERACTIVE=true
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --gpio-only     Run only GPIO tests"
            echo "  --led-only      Run only LED tests"
            echo "  --adc-only      Run only ADC tests"
            echo "  --quick         Run quick tests only"
            echo "  --interactive   Include interactive tests"
            echo "  --help          Show this help"
            exit 0
            ;;
    esac
done

# Build tests
echo -e "${YELLOW}Building test programs...${NC}"
make -s all
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Build successful!${NC}"
echo ""

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0

run_test() {
    local name=$1
    local cmd=$2

    echo -e "${CYAN}─────────────────────────────────────────${NC}"
    echo -e "${YELLOW}Running: $name${NC}"
    echo -e "${CYAN}─────────────────────────────────────────${NC}"

    eval $cmd
    local result=$?

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if [ $result -eq 0 ]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}✓ $name completed${NC}"
    else
        echo -e "${RED}✗ $name failed${NC}"
    fi
    echo ""
}

# Run GPIO tests
if [ "$RUN_GPIO" = true ]; then
    echo -e "\n${CYAN}════════════════════════════════════════${NC}"
    echo -e "${CYAN}           GPIO TESTS                   ${NC}"
    echo -e "${CYAN}════════════════════════════════════════${NC}\n"

    run_test "GPIO Export/Unexport" "./test_gpio 1"
    run_test "GPIO Direction" "./test_gpio 2"
    run_test "GPIO Value (LED Blink)" "./test_gpio 3"

    if [ "$QUICK_MODE" = false ]; then
        run_test "All Project GPIOs" "./test_gpio 4"
    fi
fi

# Run LED tests
if [ "$RUN_LED" = true ]; then
    echo -e "\n${CYAN}════════════════════════════════════════${NC}"
    echo -e "${CYAN}           LED TESTS                    ${NC}"
    echo -e "${CYAN}════════════════════════════════════════${NC}\n"

    run_test "Single LED" "./test_led 1"
    run_test "All 4 LEDs" "./test_led 2"

    if [ "$QUICK_MODE" = false ]; then
        run_test "LED Patterns" "./test_led 3"
    fi

    if [ "$INTERACTIVE" = true ]; then
        run_test "Interactive LED Test" "./test_led 4"
    fi
fi

# Run ADC tests
if [ "$RUN_ADC" = true ]; then
    echo -e "\n${CYAN}════════════════════════════════════════${NC}"
    echo -e "${CYAN}           ADC TESTS                    ${NC}"
    echo -e "${CYAN}════════════════════════════════════════${NC}\n"

    run_test "ADC Single Read" "python3 test_adc.py 1"
    run_test "ADC Continuous Read" "python3 test_adc.py 2"

    if [ "$QUICK_MODE" = false ]; then
        run_test "ADC Stability" "python3 test_adc.py 3"
        run_test "ADC All Channels" "python3 test_adc.py 5"
    fi

    if [ "$INTERACTIVE" = true ]; then
        run_test "Light Change Detection" "python3 test_adc.py 4"
    fi
fi

# Summary
echo -e "\n${CYAN}════════════════════════════════════════${NC}"
echo -e "${CYAN}           TEST SUMMARY                 ${NC}"
echo -e "${CYAN}════════════════════════════════════════${NC}"
echo ""
echo -e "Total tests run: ${TOTAL_TESTS}"
echo -e "Tests passed:    ${GREEN}${PASSED_TESTS}${NC}"
echo -e "Tests failed:    ${RED}$((TOTAL_TESTS - PASSED_TESTS))${NC}"
echo ""

if [ $PASSED_TESTS -eq $TOTAL_TESTS ]; then
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║         ALL TESTS PASSED!              ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${YELLOW}╔════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║      SOME TESTS FAILED                 ║${NC}"
    echo -e "${YELLOW}╚════════════════════════════════════════╝${NC}"
    exit 1
fi
