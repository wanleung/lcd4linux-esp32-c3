# Hardware Design Files

This directory contains all hardware design files for the ESP32-C3 OLED Display project.

## 📁 Files Overview

- **`esp32c3-oled.kicad_pro`** - KiCad project file
- **`esp32c3-oled.kicad_sch`** - KiCad schematic
- **`BOM.md`** - Complete Bill of Materials with purchase links
- **`PCB_DESIGN_GUIDE.md`** - Comprehensive PCB design guide
- **`test-pcb.py`** - PCB testing script

## 🚀 Quick Start Options

### Option 1: Use Development Boards (Easiest) ⭐

**What you need:**
```
ESP32-C3 DevKit board    →  $5-8
128x32 OLED I2C display  →  $3-5
4x Female-Female wires   →  $1
USB-C cable              →  $2
─────────────────────────
Total: $11-16
```

**Wiring:**
```
ESP32-C3    →    OLED
GPIO8 (SDA) →    SDA
GPIO9 (SCL) →    SCL
3.3V        →    VCC
GND         →    GND
```

**Time to working:** 5 minutes ⚡

### Option 2: Custom PCB (Advanced)

**What you need:**
- See [BOM.md](BOM.md) for complete parts list
- PCB from JLCPCB (~$5 for 5 boards)
- Components (~$10)
- Soldering equipment

**Time to working:** 2-3 weeks (including PCB fab time)

See [PCB_DESIGN_GUIDE.md](PCB_DESIGN_GUIDE.md) for complete instructions.

## 📦 Component Shopping

### Quick Shopping List

**Amazon (Fast shipping):**
1. Search: "ESP32-C3 development board"
2. Search: "128x32 OLED I2C SSD1306"
3. Search: "Female jumper wires"

**AliExpress (Cheaper):**
1. Search: "ESP32-C3 DevKit"
2. Search: "0.91 inch 128x32 OLED I2C"
3. Search: "Dupont jumper wire female"

See [BOM.md](BOM.md) for detailed part numbers and links.

## 🔧 Recommended Hardware

### Best ESP32-C3 Boards

**1. Seeed XIAO ESP32C3** ⭐ Recommended
- **Size:** 21x17.5mm (tiny!)
- **Price:** $4.99
- **Features:** USB-C, battery charging
- **Buy:** [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html)

**2. ESP32-C3-DevKitM-1**
- **Size:** Standard dev board
- **Price:** $5-7
- **Features:** Official Espressif board
- **Buy:** [Amazon](https://www.amazon.com/s?k=ESP32-C3-DevKitM-1)

**3. Generic ESP32-C3 Mini**
- **Price:** $2-4
- **Features:** Basic, cheap
- **Buy:** [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32-C3+mini)

### Best OLED Displays

**1. Generic 128x32 I2C** ⭐ Best Value
- **Price:** $3-5
- **Address:** 0x3C
- **Buy:** [Amazon](https://www.amazon.com/s?k=128x32+oled+i2c)

**2. Adafruit 128x32 OLED** (Quality)
- **Part:** #4440
- **Price:** $14.95
- **Buy:** [Adafruit](https://www.adafruit.com/product/4440)

## 🧪 Testing Your Hardware

### Quick Test

```bash
# Install dependencies
pip3 install pyserial

# Run test
cd hardware
./test-pcb.py --port /dev/ttyACM0
```

### Manual Test

```bash
# Open serial monitor at 115200 baud
# Type these commands:

PING       # Should return "OK:PONG"
INFO       # Should return display info
CLEAR      # Clears display
TEXT:0:0:1:Hello
DISPLAY    # Updates screen
```

## 📐 Circuit Schematic

```
                +3.3V
                  │
    ┌─────────────┴──────────────┐
    │                            │
    │    ESP32-C3-WROOM-02       │
    │                            │
    │  GPIO8(SDA)    GPIO9(SCL)  │
    └────┬──────────────┬────────┘
         │              │
         │4.7kΩ*     4.7kΩ*  (* Optional pull-ups)
         │              │
    ┌────┴──────────────┴────────┐
    │   OLED 128x32 I2C          │
    │   SSD1306                  │
    │   SDA    SCL   VCC   GND   │
    └──────────────────┬─────┬───┘
                       │     │
                      +3.3V GND
```

## 🔌 Pin Mapping

| Signal | ESP32-C3 Pin | OLED Pin | Note |
|--------|--------------|----------|------|
| I2C Data | GPIO8 | SDA | Internal pull-up enabled |
| I2C Clock | GPIO9 | SCL | Internal pull-up enabled |
| Power | 3.3V | VCC | 10-20mA typical |
| Ground | GND | GND | Common ground |
| USB Data- | GPIO18 | - | For serial comm |
| USB Data+ | GPIO19 | - | For serial comm |

## 🎨 PCB Design

### Using KiCad

```bash
# Install KiCad
sudo apt install kicad
# or
flatpak install flathub org.kicad.KiCad

# Open project
cd hardware
kicad esp32c3-oled.kicad_pro
```

### Design Guidelines

- **Board size:** 40x60mm recommended
- **Layers:** 2 (Top + Bottom)
- **Track width:** 0.5mm for power, 0.25mm for signals
- **Keep I2C traces short**
- **Ground plane on bottom layer**

See [PCB_DESIGN_GUIDE.md](PCB_DESIGN_GUIDE.md) for complete guide.

## 📤 PCB Manufacturing

### JLCPCB (Recommended)

1. **Export Gerbers** from KiCad
2. **Upload** to [JLCPCB](https://jlcpcb.com)
3. **Order:** $2 for 5 PCBs (100x100mm)
4. **Wait:** 7-14 days with shipping

### Specifications

```
Layers: 2
Thickness: 1.6mm
Copper: 1oz (35µm)
Surface: HASL or ENIG
Color: Any (green standard)
Min Track/Space: 6/6 mil
```

## 🔍 Troubleshooting

### Display not working

**Check:**
- [ ] I2C address (0x3C or 0x3D)
- [ ] SDA/SCL not swapped
- [ ] 3.3V power is present
- [ ] GND is connected
- [ ] No solder bridges

**Test:**
```bash
# Run I2C scanner (in Arduino IDE)
# Should detect device at 0x3C
```

### ESP32 not recognized

**Check:**
- [ ] USB cable is data-capable (not charge-only)
- [ ] "USB CDC On Boot" enabled in Arduino
- [ ] D+/D- connected to GPIO18/19
- [ ] 3.3V power stable

**Test:**
```bash
ls /dev/ttyACM*  # Should show device
dmesg | tail     # Check USB enumeration
```

### Communication errors

**Check:**
- [ ] Baud rate is 115200
- [ ] Port has correct permissions
- [ ] No other program using port
- [ ] ESP32 sketch uploaded correctly

**Fix:**
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

## 📚 Resources

### Datasheets
- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [ESP32-C3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [SSD1306 OLED Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

### Tutorials
- [KiCad Getting Started](https://docs.kicad.org/master/en/getting_started_in_kicad/)
- [PCB Design Tutorial](https://www.youtube.com/watch?v=35YuILUlfGs)
- [JLCPCB Ordering Guide](https://www.youtube.com/watch?v=C7-8nUU6e3E)

### Libraries
- [ESP32 KiCad Library](https://github.com/espressif/kicad-libraries)
- [Adafruit KiCad Library](https://github.com/adafruit/Adafruit-KiCad-Library)

## 🤝 Contributing

Have a better PCB design? Found a cheaper component source? Please contribute!

1. Fork the repository
2. Create your design
3. Submit a pull request

## 📝 License

Hardware designs are released under CERN-OHL-P v2 (permissive).

You are free to:
- Use commercially
- Modify and redistribute
- Create derivative works

## 🆘 Getting Help

**Having issues?**
1. Check [PCB_DESIGN_GUIDE.md](PCB_DESIGN_GUIDE.md) troubleshooting section
2. Run `./test-pcb.py` for diagnostics
3. Review wiring diagram above
4. Ask in project issues

## ✅ Next Steps

1. **Choose your path:**
   - Quick: Buy dev boards and wire them
   - Advanced: Design and order custom PCB

2. **Order components** (see [BOM.md](BOM.md))

3. **While waiting:**
   - Upload Arduino sketch
   - Test with Python bridge
   - Read PCB design guide

4. **On arrival:**
   - Assemble/connect hardware
   - Run `./test-pcb.py`
   - Deploy LCD4Linux!

Happy building! 🎉
