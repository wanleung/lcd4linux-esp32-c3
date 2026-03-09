#!/bin/bash
# Helper script to integrate ESP32OLED driver into lcd4linux source
# Usage: ./integrate-driver.sh /path/to/lcd4linux/source

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

LCD4LINUX_DIR="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${BLUE}=== ESP32OLED Driver Integration Script ===${NC}"
echo

# Check if lcd4linux directory provided
if [ -z "$LCD4LINUX_DIR" ]; then
    echo -e "${RED}Error: Please provide lcd4linux source directory${NC}"
    echo "Usage: $0 /path/to/lcd4linux"
    echo
    echo "To download lcd4linux:"
    echo "  git clone https://github.com/TangoCash/lcd4linux.git"
    exit 1
fi

# Check if directory exists
if [ ! -d "$LCD4LINUX_DIR" ]; then
    echo -e "${RED}Error: Directory not found: $LCD4LINUX_DIR${NC}"
    exit 1
fi

# Check if it looks like lcd4linux source
if [ ! -f "$LCD4LINUX_DIR/configure.ac" ] && [ ! -f "$LCD4LINUX_DIR/configure.in" ]; then
    echo -e "${RED}Error: Not a valid lcd4linux source directory${NC}"
    echo "Missing configure.ac or configure.in"
    exit 1
fi

echo -e "${GREEN}✓ Found lcd4linux source${NC}"
echo

# Step 1: Copy driver file
echo -e "${BLUE}[1/4] Copying driver source...${NC}"
cp -v "$SCRIPT_DIR/driver/drv_ESP32OLED.c" "$LCD4LINUX_DIR/"
echo -e "${GREEN}✓ Driver file copied${NC}"
echo

# Step 2: Modify drv.c
echo -e "${BLUE}[2/4] Modifying drv.c...${NC}"
DRV_C="$LCD4LINUX_DIR/drv.c"

if ! grep -q "WITH_ESP32OLED" "$DRV_C"; then
    # Find the line with "extern DRIVER drv_" declarations
    LINE=$(grep -n "extern DRIVER" "$DRV_C" | tail -1 | cut -d: -f1)
    
    # Add extern declaration
    sed -i "${LINE}a\\
#ifdef WITH_ESP32OLED\\
extern DRIVER drv_ESP32OLED;\\
#endif" "$DRV_C"
    
    # Find the driver list array
    LINE=$(grep -n "DRIVER \*Driver\[\]" "$DRV_C" | cut -d: -f1)
    LINE=$((LINE + 1))
    
    # Add to driver list
    sed -i "${LINE}a\\
#ifdef WITH_ESP32OLED\\
    \&drv_ESP32OLED,\\
#endif" "$DRV_C"
    
    echo -e "${GREEN}✓ drv.c modified${NC}"
else
    echo -e "${YELLOW}! drv.c already contains ESP32OLED${NC}"
fi
echo

# Step 3: Modify configure
echo -e "${BLUE}[3/4] Modifying configure script...${NC}"

if [ -f "$LCD4LINUX_DIR/configure.ac" ]; then
    CONFIGURE_FILE="$LCD4LINUX_DIR/configure.ac"
elif [ -f "$LCD4LINUX_DIR/configure.in" ]; then
    CONFIGURE_FILE="$LCD4LINUX_DIR/configure.in"
else
    echo -e "${RED}Error: No configure.ac or configure.in found${NC}"
    exit 1
fi

if ! grep -q "with-esp32oled" "$CONFIGURE_FILE"; then
    # Find a good place to insert (after another driver definition)
    LINE=$(grep -n "with-.*driver" "$CONFIGURE_FILE" | tail -1 | cut -d: -f1)
    
    # Add ESP32OLED driver configuration
    sed -i "${LINE}a\\
\\
# ESP32-OLED driver\\
AC_ARG_WITH(\\
  esp32oled,\\
  [  --with-esp32oled        ESP32-C3 with I2C OLED display [[default=yes]]],\\
  with_esp32oled=\"\$withval\",\\
  with_esp32oled=\"yes\"\\
)\\
if test \"\$with_esp32oled\" = \"yes\"; then\\
  TEXT=\"yes\"\\
  GRAPHIC=\"yes\"\\
  SERIAL=\"yes\"\\
  DRIVERS=\"\$DRIVERS drv_ESP32OLED.o\"\\
  AC_DEFINE(WITH_ESP32OLED,1,[ESP32-C3 OLED driver])\\
fi" "$CONFIGURE_FILE"
    
    echo -e "${GREEN}✓ Configure script modified${NC}"
else
    echo -e "${YELLOW}! Configure already contains ESP32OLED${NC}"
fi
echo

# Step 4: Instructions
echo -e "${BLUE}[4/4] Next steps:${NC}"
echo
echo "1. Regenerate configure script:"
echo -e "   ${YELLOW}cd $LCD4LINUX_DIR${NC}"
echo -e "   ${YELLOW}./bootstrap${NC}"
echo "   or:"
echo -e "   ${YELLOW}autoreconf -fi${NC}"
echo
echo "2. Configure with ESP32OLED driver:"
echo -e "   ${YELLOW}./configure --with-esp32oled --prefix=/usr/local${NC}"
echo
echo "3. Build:"
echo -e "   ${YELLOW}make -j\$(nproc)${NC}"
echo
echo "4. Verify driver is included:"
echo -e "   ${YELLOW}./lcd4linux -l | grep ESP32${NC}"
echo
echo "5. Install:"
echo -e "   ${YELLOW}sudo make install${NC}"
echo
echo "6. Configure and test:"
echo -e "   ${YELLOW}sudo cp $SCRIPT_DIR/lcd4linux-native.conf /etc/lcd4linux.conf${NC}"
echo -e "   ${YELLOW}sudo nano /etc/lcd4linux.conf  # Edit Port setting${NC}"
echo -e "   ${YELLOW}sudo lcd4linux -F -f /etc/lcd4linux.conf${NC}"
echo

echo -e "${GREEN}=== Integration complete! ===${NC}"
echo
echo "For detailed instructions, see:"
echo "  $SCRIPT_DIR/COMPILE_NATIVE.md"
