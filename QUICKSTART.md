# Quick Start Guide

## 1. Hardware Setup
1. Connect OLED display to ESP32-C3:
   - VCC → 3.3V
   - GND → GND  
   - SDA → GPIO8
   - SCL → GPIO9

2. Connect ESP32-C3 to computer via USB

## 2. Arduino Setup
1. Open Arduino IDE
2. Install ESP32 board support (see README.md)
3. Install libraries:
   - Adafruit GFX Library
   - Adafruit SSD1306
4. Open `lcd4linux_esp32.ino`
5. Edit `config.h` if using different pins
6. Select board: ESP32C3 Dev Module
7. Enable "USB CDC On Boot"
8. Upload to ESP32-C3

## 3. Test Connection
```bash
# Install Python dependencies
pip3 install -r requirements.txt

# Run test script
./test-connection.sh
```

## 4. Run Display
```bash
# System monitor mode
./lcd4linux-bridge.py --port /dev/ttyACM0 --mode system

# Test pattern
./lcd4linux-bridge.py --port /dev/ttyACM0 --mode test
```

## 5. Install as Service (Optional)
```bash
# Edit service file with your paths
sudo nano lcd4linux-esp32.service

# Install service
sudo cp lcd4linux-esp32.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable lcd4linux-esp32.service
sudo systemctl start lcd4linux-esp32.service

# Check status
sudo systemctl status lcd4linux-esp32.service
```

## Troubleshooting
- **Port not found**: Check `ls /dev/ttyACM*` or `ls /dev/ttyUSB*`
- **Permission denied**: Add user to dialout group: `sudo usermod -a -G dialout $USER`
- **Display not working**: Verify I2C address (try 0x3C or 0x3D in config.h)
- **ESP32 not recognized**: Enable "USB CDC On Boot" in Arduino IDE

## File Overview
- `lcd4linux_esp32.ino` - Main Arduino sketch
- `config.h` - Pin configuration
- `lcd4linux-bridge.py` - Python bridge for system stats
- `test-connection.sh` - Connection test script
- `lcd4linux.conf` - Example lcd4linux config
- `README.md` - Full documentation
