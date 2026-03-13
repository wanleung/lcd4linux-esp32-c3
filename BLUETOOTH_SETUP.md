# Bluetooth BLE Setup Guide

This guide explains how to set up and use Bluetooth BLE connectivity with your LCD4Linux ESP32-C3 display.

## Overview

Your ESP32-C3 now supports **dual connectivity**:
- **USB Serial** (original method)
- **Bluetooth BLE** (new wireless method)

Both can work simultaneously, allowing you to control the display from either connection.

## Hardware Requirements

- ESP32-C3 with Bluetooth LE support (built-in on ESP32-C3)
- Computer with Bluetooth adapter (Linux)
- OLED display (I2C SSD1306)

## ESP32 Firmware

### Arduino Libraries Required

Make sure you have these libraries installed in Arduino IDE:
1. **Adafruit GFX Library**
2. **Adafruit SSD1306**
3. **ESP32 BLE Arduino** (comes with ESP32 board package)

### Flashing the Firmware

1. Open `lcd4linux_esp32.ino` in Arduino IDE
2. Select **Board**: ESP32C3 Dev Module
3. Configure Board Settings:
   - USB CDC On Boot: Enabled
   - Flash Mode: QIO
   - Flash Size: 4MB
   - Partition Scheme: Default
4. Upload the sketch

### Configuration

Edit [config.h](config.h) to customize:

```cpp
// BLE device name (shows up when scanning)
#define BLE_DEVICE_NAME "LCD4Linux-ESP32"

// I2C pins
#define I2C_SDA 3
#define I2C_SCL 4
```

## Linux Setup

### 1. Install Dependencies

```bash
# Install Python Bluetooth library
pip3 install -r requirements.txt

# Or install manually:
pip3 install bleak>=0.21.0
```

### 2. Ensure Bluetooth is Working

```bash
# Check Bluetooth service
sudo systemctl status bluetooth

# Start if not running
sudo systemctl start bluetooth

# Enable Bluetooth
bluetoothctl power on
```

### 3. Find Your ESP32 Device

Scan for BLE devices to verify your ESP32 is advertising:

```bash
sudo hcitool lescan
# Or using bluetoothctl:
bluetoothctl
scan on
```

You should see `LCD4Linux-ESP32` in the scan results.

## Using the Bluetooth Daemon

### Basic Usage

The `lcd4linux-bt-daemon.py` script connects to your ESP32 via BLE.

#### System Info Mode (Default)
Displays CPU, memory, and time continuously:

```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode system
```

#### Custom Text Mode
Enter custom text to display:

```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode custom
```

#### Test Pattern
Display a test pattern once:

```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode test
```

### Connection Options

**By device name (recommended):**
```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32
```

**By MAC address:**
```bash
# First, find the MAC address
sudo hcitool lescan

# Then connect using address
./lcd4linux-bt-daemon.py --address AA:BB:CC:DD:EE:FF
```

### Running as Systemd Service

To run the Bluetooth daemon automatically at boot:

#### 1. Edit the service file

Edit [lcd4linux-bt-daemon.service](lcd4linux-bt-daemon.service) and update paths if needed:

```ini
ExecStart=/usr/bin/python3 /home/YOUR_USERNAME/Projects/lcd4linux_esp32/lcd4linux-bt-daemon.py --mode daemon --name LCD4Linux-ESP32
```

#### 2. Install the service

```bash
# Copy service file to systemd directory
sudo cp lcd4linux-bt-daemon.service /etc/systemd/system/lcd4linux-bt-daemon@.service

# Reload systemd
sudo systemctl daemon-reload

# Enable and start for your user
sudo systemctl enable lcd4linux-bt-daemon@$USER.service
sudo systemctl start lcd4linux-bt-daemon@$USER.service

# Check status
sudo systemctl status lcd4linux-bt-daemon@$USER.service
```

#### 3. View logs

```bash
# Follow logs in real-time
journalctl -u lcd4linux-bt-daemon@$USER.service -f

# View recent logs
journalctl -u lcd4linux-bt-daemon@$USER.service -n 50
```

## Python API Usage

You can also use the Bluetooth daemon in your own Python scripts:

```python
import asyncio
from lcd4linux_bt_daemon import ESP32BLEDisplay, draw_system_info

async def main():
    # Connect to ESP32
    display = ESP32BLEDisplay(device_name='LCD4Linux-ESP32')
    await display.connect()
    
    # Clear display
    await display.clear()
    
    # Draw text
    await display.text(0, 0, 1, "Hello from BLE!")
    await display.text(0, 10, 1, "Line 2")
    
    # Update display
    await display.display()
    
    # Draw shapes
    await display.line(0, 20, 127, 20)
    await display.rect(10, 22, 30, 8, fill=0)
    await display.display()
    
    # Disconnect
    await display.disconnect()

if __name__ == '__main__':
    asyncio.run(main())
```

## Command Protocol

Both Serial and BLE use the same text-based command protocol:

| Command | Format | Description |
|---------|--------|-------------|
| CLEAR | `CLEAR` | Clear display buffer |
| TEXT | `TEXT:x:y:size:text` | Draw text at (x,y) |
| PIXEL | `PIXEL:x:y:state` | Draw pixel at (x,y) |
| LINE | `LINE:x0:y0:x1:y1` | Draw line |
| RECT | `RECT:x:y:w:h:fill` | Draw rectangle |
| CIRCLE | `CIRCLE:x:y:r:fill` | Draw circle |
| DISPLAY | `DISPLAY` | Update physical display |
| PING | `PING` | Keep-alive check |
| INFO | `INFO` | Get device info |

All commands return `OK:COMMAND` on success or `ERR:DESCRIPTION` on error.

## Troubleshooting

### Cannot find ESP32 device

1. **Check ESP32 is powered and running:**
   - Look at the OLED - it should show "LCD4Linux Ready" with "Serial + BLE"
   
2. **Check Bluetooth is enabled:**
   ```bash
   sudo systemctl status bluetooth
   bluetoothctl power on
   ```

3. **Increase scan timeout:**
   ```bash
   ./lcd4linux-bt-daemon.py --scan-timeout 30.0
   ```

4. **Check device name in config.h matches:**
   - Default is `LCD4Linux-ESP32`

### Connection drops frequently

1. **Check distance** - BLE works best within ~10 meters
2. **Reduce interference** - Move away from WiFi routers, microwaves
3. **Check power supply** - Ensure ESP32 has stable power

### Permission errors on Linux

If you get permission errors:

```bash
# Add user to bluetooth group
sudo usermod -a -G bluetooth $USER

# Log out and back in for changes to take effect
```

Or run with sudo (not recommended for security):
```bash
sudo python3 ./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32
```

### ESP32 not responding

1. **Try the PING command:**
   ```python
   await display.ping()
   ```

2. **Check Serial monitor in Arduino IDE** for debug messages

3. **Restart ESP32** - power cycle or press reset button

## Comparing Serial vs BLE

### USB Serial (Original)
**Pros:**
- Fast and reliable
- No pairing required
- Lower latency

**Cons:**
- Requires USB cable
- Device must be physically connected

### Bluetooth BLE (New)
**Pros:**
- Wireless - no cables needed
- Works from distance (~10m)
- Can move display freely

**Cons:**
- Slightly higher latency
- Requires Bluetooth setup
- Can have interference issues

## Advanced Configuration

### Change BLE Device Name

Edit [config.h](config.h):
```cpp
#define BLE_DEVICE_NAME "MyCustomName"
```

Re-upload the sketch to ESP32.

### Using Multiple Displays

Each ESP32 needs a unique name:

**ESP32 #1:**
```cpp
#define BLE_DEVICE_NAME "LCD4Linux-Display1"
```

**ESP32 #2:**
```cpp
#define BLE_DEVICE_NAME "LCD4Linux-Display2"
```

Connect to each:
```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-Display1
./lcd4linux-bt-daemon.py --name LCD4Linux-Display2
```

## Security Notes

- BLE connections in this implementation are **not encrypted**
- Only use on trusted networks
- Commands are sent in plain text
- Consider this for non-sensitive information display only

## Next Steps

- See [QUICKSTART.md](QUICKSTART.md) for general setup guide
- See [README.md](README.md) for project overview
- Check [lcd4linux.conf](lcd4linux.conf) for LCD4Linux native integration

---

**Enjoy your wireless display! 🎉**
