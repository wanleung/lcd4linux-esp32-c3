# LCD4Linux ESP32-C3 Driver

Arduino project that enables ESP32-C3 to act as a display driver for lcd4linux daemon using an I2C OLED 128x32 display (SSD1306).

## Connectivity Options

This project supports **dual connectivity**:

### 🔌 USB Serial (Default)
- Fast and reliable wired connection
- Plug-and-play via USB cable
- Lower latency

### 📡 Bluetooth BLE (Wireless)
- **NEW!** Control display wirelessly via Bluetooth
- No cables needed - works from ~10m distance
- Same command protocol as Serial
- 📖 Setup guide: [BLUETOOTH_SETUP.md](BLUETOOTH_SETUP.md)

**Both connections work simultaneously!**

## Two Integration Options

This project offers **two ways** to display system information:

### Option A: Python Bridge (Recommended for Quick Start)
- ✅ **Easy setup** - No compilation required
- ✅ **Quick to customize** - Edit Python script directly
- ✅ **Works immediately** - Just install Python dependencies
- 📁 Uses: `lcd4linux-bridge.py` (Serial) or `lcd4linux-bt-daemon.py` (Bluetooth)
- 📖 Guide: [QUICKSTART.md](QUICKSTART.md) | [BLUETOOTH_SETUP.md](BLUETOOTH_SETUP.md)

### Option B: Native LCD4Linux Driver (Advanced)
- ✅ **Lower CPU usage** - Native C code
- ✅ **Full lcd4linux integration** - All widgets & plugins
- ✅ **Standard daemon** - System service integration
- ⚠️ **Requires compilation** - Build lcd4linux from source
- 📁 Uses: `driver/drv_ESP32OLED.c`
- 📖 Guide: [COMPILE_NATIVE.md](COMPILE_NATIVE.md)

**New users:** Start with **Option A** (Python Bridge). Switch to Option B later if you need native lcd4linux integration.

## Hardware Requirements

- **ESP32-C3** development board (with USB support)
- **128x32 I2C OLED Display** (SSD1306 controller)
- **Wiring:**
  - OLED VCC → 3.3V
  - OLED GND → GND
  - OLED SDA → GPIO8 (configurable in `config.h`)
  - OLED SCL → GPIO9 (configurable in `config.h`)

📦 **Need to buy components?** See [hardware/BOM.md](hardware/BOM.md) for complete shopping list with links!

🔧 **Want to design a custom PCB?** See [hardware/PCB_DESIGN_GUIDE.md](hardware/PCB_DESIGN_GUIDE.md) for KiCad design files and manufacturing guide!

## Software Requirements

### Arduino IDE Setup

1. **Install Arduino IDE** (version 1.8.x or 2.x)

2. **Add ESP32 Board Support:**
   - Open Arduino IDE → File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

3. **Install Required Libraries:**
   - Go to Tools → Manage Libraries
   - Search and install:
     - **Adafruit GFX Library** by Adafruit
     - **Adafruit SSD1306** by Adafruit

### Board Configuration

- Tools → Board → ESP32 Arduino → **ESP32C3 Dev Module**
- Tools → USB CDC On Boot → **Enabled**
- Tools → Port → Select your ESP32-C3 port (e.g., `/dev/ttyACM0` on Linux)

## Configuration

Edit the `config.h` file to customize pin settings:

```cpp
// I2C Pin Configuration
#define I2C_SDA 8    // Data line (GPIO8)
#define I2C_SCL 9    // Clock line (GPIO9)

// Display Settings
#define SCREEN_ADDRESS 0x3C  // I2C address (0x3C or 0x3D)
#define I2C_FREQUENCY 400000 // I2C clock speed
```

## Installation

1. **Clone or download this project**
2. **Open `lcd4linux_esp32.ino` in Arduino IDE**
3. **Configure settings** in `config.h` if needed
4. **Select ESP32-C3 board** and port
5. **Upload** the sketch to your ESP32-C3

## Testing Connection

After uploading, open the Serial Monitor (115200 baud):
- You should see: `OK:READY`
- The OLED should display "LCD4Linux Ready"

Test commands manually in Serial Monitor:
```
INFO        → Returns display info
CLEAR       → Clears the display
TEXT:0:0:1:Hello World → Displays text
DISPLAY     → Updates the screen
PING        → Returns PONG
```

## Protocol Reference

Communication format: `COMMAND:param1:param2:...\n`

### Supported Commands

| Command | Parameters | Description |
|---------|------------|-------------|
| `CLEAR` | none | Clear display buffer |
| `TEXT` | x:y:size:text | Draw text at (x,y) with size |
| `PIXEL` | x:y:state | Set pixel (state: 0=off, 1=on) |
| `LINE` | x0:y0:x1:y1 | Draw line from (x0,y0) to (x1,y1) |
| `RECT` | x:y:w:h:fill | Draw rectangle (fill: 0=outline, 1=filled) |
| `CIRCLE` | x:y:r:fill | Draw circle (fill: 0=outline, 1=filled) |
| `DISPLAY` | none | Update physical display with buffer |
| `PING` | none | Keep-alive check (returns PONG) |
| `INFO` | none | Get display dimensions and pin info |

### Response Format

- Success: `OK:COMMAND` or `OK:DATA`
- Error: `ERR:ERROR_TYPE`

## Using the Display

### Option A: Python Bridge (Quick Start)

```bash
# Install dependencies
pip3 install -r requirements.txt

# Run system monitor
./lcd4linux-bridge.py --port /dev/ttyACM0 --mode system

# Run as background service
sudo cp lcd4linux-esp32.service /etc/systemd/system/
sudo systemctl enable --now lcd4linux-esp32.service
```

See [QUICKSTART.md](QUICKSTART.md) for detailed instructions.

### Option B: Native LCD4Linux Driver (Advanced)

**Full lcd4linux integration** with native C driver:

```bash
# 1. Download lcd4linux source
git clone https://github.com/TangoCash/lcd4linux.git

# 2. Integrate ESP32OLED driver
./integrate-driver.sh /path/to/lcd4linux

# 3. Follow compilation instructions
# See COMPILE_NATIVE.md for complete guide

# 4. Use native configuration
sudo cp lcd4linux-native.conf /etc/lcd4linux.conf
sudo lcd4linux -F -f /etc/lcd4linux.conf
```

See [COMPILE_NATIVE.md](COMPILE_NATIVE.md) for complete compilation guide.

## Troubleshooting

### Display not working
- Check I2C address (try 0x3C or 0x3D)
- Verify wiring (SDA/SCL not swapped)
- Test I2C with scanner sketch

### Serial port not found
- Ensure "USB CDC On Boot" is **Enabled** in Arduino IDE
- Check if port appears: `ls /dev/ttyACM*` or `ls /dev/ttyUSB*`
- Add user to dialout group: `sudo usermod -a -G dialout $USER`

### ESP32-C3 not recognized
- Install CH340/CP210x USB drivers if needed
- Try pressing BOOT button while connecting USB
- Update ESP32 board package in Arduino IDE

### LCD4Linux connection issues
- Verify baud rate matches (115200)
- Check serial port permissions
- Monitor serial output: `cat /dev/ttyACM0`

## Pin Reference

ESP32-C3 available GPIO pins:
- GPIO0-10, GPIO18-21
- **Avoid GPIO18/19** (USB D-/D+) if using USB Serial
- Recommended I2C: GPIO8 (SDA), GPIO9 (SCL)

## License

This project is provided as-is for use with lcd4linux.

## Credits

- Uses Adafruit GFX and SSD1306 libraries
- Designed for lcd4linux daemon compatibility
