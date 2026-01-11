#!/bin/bash
#
# Build Verification Script
# Verifies that the Qt project can be built successfully
#

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build_test"

echo -e "${CYAN}╔════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║      Build Verification Script         ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════╝${NC}"
echo ""

# Cleanup previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${YELLOW}Step 1: Checking required files...${NC}"
REQUIRED_FILES=(
    "main.cpp"
    "mainwindow.h"
    "mainwindow.cpp"
    "mainwindow.ui"
    "final_project.pro"
    "gpio_util.cpp"
    "gpio_util.hpp"
    "gpio_LED_CTRL.cpp"
    "gpio_LED_CTRL.hpp"
    "PIN_LOOKUP.hpp"
    "read_adc.py"
    "qtres.qrc"
    "lightbulb.png"
)

all_files_exist=true
for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$PROJECT_DIR/$file" ]; then
        echo -e "  ${GREEN}✓${NC} $file"
    else
        echo -e "  ${RED}✗${NC} $file - MISSING!"
        all_files_exist=false
    fi
done

if [ "$all_files_exist" = false ]; then
    echo -e "\n${RED}Missing required files!${NC}"
    exit 1
fi
echo -e "${GREEN}All required files present.${NC}\n"

echo -e "${YELLOW}Step 2: Running qmake...${NC}"
qmake "$PROJECT_DIR/final_project.pro" -spec linux-g++ CONFIG+=debug CONFIG+=qml_debug
if [ $? -ne 0 ]; then
    echo -e "${RED}qmake failed!${NC}"
    exit 1
fi
echo -e "${GREEN}qmake successful.${NC}\n"

echo -e "${YELLOW}Step 3: Compiling project...${NC}"
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful.${NC}\n"

echo -e "${YELLOW}Step 4: Verifying executable...${NC}"
if [ -f "$BUILD_DIR/final_project" ]; then
    echo -e "  ${GREEN}✓${NC} Executable created: $BUILD_DIR/final_project"
    file "$BUILD_DIR/final_project"
else
    echo -e "  ${RED}✗${NC} Executable not found!"
    exit 1
fi

echo ""
echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║      BUILD VERIFICATION PASSED!        ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo ""
echo "To run the application:"
echo "  cd $BUILD_DIR"
echo "  sudo ./final_project"
echo ""
echo "Or clean up with:"
echo "  rm -rf $BUILD_DIR"

exit 0
