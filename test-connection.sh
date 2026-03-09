#!/bin/bash
# Test script for LCD4Linux ESP32-C3 setup
# This script helps verify that the ESP32-C3 is properly connected and responding

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== LCD4Linux ESP32-C3 Test Script ==="
echo

# Check if pyserial is installed
echo -n "Checking for pyserial... "
if python3 -c "import serial" 2>/dev/null; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}NOT FOUND${NC}"
    echo "Install with: pip3 install pyserial"
    exit 1
fi

# Check if psutil is installed
echo -n "Checking for psutil... "
if python3 -c "import psutil" 2>/dev/null; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}NOT FOUND${NC}"
    echo "Install with: pip3 install psutil"
    exit 1
fi

# Find available serial ports
echo
echo "Available serial ports:"
ports=$(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null)
if [ -z "$ports" ]; then
    echo -e "${RED}No serial ports found!${NC}"
    echo "Make sure your ESP32-C3 is connected."
    exit 1
fi

for port in $ports; do
    echo "  $port"
done

# Prompt for port selection
echo
if [ -n "$1" ]; then
    PORT="$1"
    echo "Using specified port: $PORT"
else
    read -p "Enter serial port [/dev/ttyACM0]: " PORT
    PORT=${PORT:-/dev/ttyACM0}
fi

# Check if port exists
if [ ! -e "$PORT" ]; then
    echo -e "${RED}Error: Port $PORT not found!${NC}"
    exit 1
fi

# Check permissions
if [ ! -r "$PORT" ] || [ ! -w "$PORT" ]; then
    echo -e "${YELLOW}Warning: You may not have permission to access $PORT${NC}"
    echo "Try: sudo usermod -a -G dialout $USER"
    echo "Then log out and log back in."
fi

# Test connection
echo
echo "Testing connection to ESP32-C3 on $PORT..."
echo

# Send PING command
response=$(echo "PING" | timeout 2 python3 -c "
import serial
import sys
import time

try:
    ser = serial.Serial('$PORT', 115200, timeout=2)
    time.sleep(2)  # Wait for ESP32 to reset
    
    # Clear any startup messages
    while ser.in_waiting:
        ser.readline()
    
    # Send PING
    ser.write(b'PING\n')
    ser.flush()
    
    # Read response
    response = ser.readline().decode('utf-8').strip()
    print(response)
    
    ser.close()
except Exception as e:
    print(f'ERROR: {e}', file=sys.stderr)
    sys.exit(1)
" 2>&1)

if echo "$response" | grep -q "PONG"; then
    echo -e "${GREEN}✓ Connection successful!${NC}"
    echo "Response: $response"
else
    echo -e "${RED}✗ Connection failed!${NC}"
    echo "Response: $response"
    echo
    echo "Troubleshooting:"
    echo "1. Check that the Arduino sketch is uploaded to ESP32-C3"
    echo "2. Verify USB cable is connected"
    echo "3. Check baud rate is set to 115200"
    echo "4. Try pressing the reset button on ESP32-C3"
    exit 1
fi

# Test INFO command
echo
echo "Getting display info..."
info=$(echo "INFO" | timeout 2 python3 -c "
import serial
import sys
import time

try:
    ser = serial.Serial('$PORT', 115200, timeout=2)
    time.sleep(0.5)
    
    # Clear buffer
    while ser.in_waiting:
        ser.readline()
    
    # Send INFO
    ser.write(b'INFO\n')
    ser.flush()
    
    # Read response
    response = ser.readline().decode('utf-8').strip()
    print(response)
    
    ser.close()
except Exception as e:
    print(f'ERROR: {e}', file=sys.stderr)
    sys.exit(1)
" 2>&1)

if echo "$info" | grep -q "OK:INFO"; then
    echo -e "${GREEN}✓ Display info received${NC}"
    echo "Info: $info"
else
    echo -e "${YELLOW}! Could not get display info${NC}"
    echo "Response: $info"
fi

# Run test pattern
echo
read -p "Display test pattern? (y/n) [y]: " SHOW_TEST
SHOW_TEST=${SHOW_TEST:-y}

if [ "$SHOW_TEST" = "y" ]; then
    echo "Displaying test pattern..."
    ./lcd4linux-bridge.py --port "$PORT" --mode test --once
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Test pattern displayed successfully!${NC}"
    else
        echo -e "${RED}✗ Failed to display test pattern${NC}"
    fi
fi

# Suggest next steps
echo
echo "=== Test Complete ==="
echo
echo "Next steps:"
echo "1. Run system monitor: ./lcd4linux-bridge.py --port $PORT --mode system"
echo "2. Install as service: sudo cp lcd4linux-esp32.service /etc/systemd/system/"
echo "3. Check README.md for full documentation"
