# PCB Design Guide - ESP32-C3 OLED Display

This guide helps you design a custom PCB for the ESP32-C3 OLED display project.

## Quick Wiring (Breadboard/Dev Board)

### Simple 4-Wire Connection

```
ESP32-C3          OLED Display
[GPIO8 (SDA)] --> [SDA]
[GPIO9 (SCL)] --> [SCL]
[3.3V]        --> [VCC]
[GND]         --> [GND]
```

### Pin Mapping Table

| ESP32-C3 Pin | OLED Pin | Purpose | Notes |
|--------------|----------|---------|-------|
| GPIO8 | SDA | I2C Data | Pull-up via internal resistor |
| GPIO9 | SCL | I2C Clock | Pull-up via internal resistor |
| 3.3V | VCC | Power | 10-20mA typical |
| GND | GND | Ground | Common ground |

**Note:** Most OLED modules have built-in pull-up resistors, so external ones are optional.

## ASCII Art Wiring Diagram

```
   ┌─────────────────────┐
   │   ESP32-C3 DevKit   │
   │                     │
   │  ┌──┐  USB  ┌──┐   │
   │  └──┘       └──┘   │
   │                     │
   │ [3V3] ●             │ ──────┐
   │ [GND] ●             │ ────┐ │
   │ [GP8] ●             │ ──┐ │ │
   │ [GP9] ●             │ ┐ │ │ │
   │                     │ │ │ │ │
   └─────────────────────┘ │ │ │ │
                           │ │ │ │
                           │ │ │ │
   ┌───────────────────┐   │ │ │ │
   │  OLED 128x32 I2C  │   │ │ │ │
   │  ┌─────────────┐  │   │ │ │ │
   │  │ ░░░░░░░░░░  │  │   │ │ │ │
   │  │ ░░ LCD4  ░░ │  │   │ │ │ │
   │  │ ░░LINUX  ░░ │  │   │ │ │ │
   │  └─────────────┘  │   │ │ │ │
   │                   │   │ │ │ │
   │  [GND] [VCC]      │   │ │ │ │
   │  [SCL] [SDA]      │   │ │ │ │
   │    ●     ●         │   │ │ │ │
   │    ●     ●         │   │ │ │ │
   └────┼─────┼─────────┘   │ │ │ │
        │     │             │ │ │ │
        └─────┼─────────────┘ │ │ │
              └───────────────┘ │ │
                └───────────────┘ │
                  └───────────────┘
```

## Schematic Design

### Complete Circuit Schematic

```
                                    +3.3V
                                      │
                                      ├─────┐
                    ┌─────────────────┤  C1 │ 10µF
                    │                 └──┬──┘
                    │                    │
                    │                   GND
    ┌───────────────┴──────────────────────┐
    │                                       │
    │          ESP32-C3-WROOM-02           │
    │                                       │
    │  3V3  GND  IO8(SDA)  IO9(SCL)  IO18  │ IO19
    └───┬───┬─────┬──────────┬───────┬─────┴──┬────┐
        │   │     │          │       │        │    │
        │   │   ┌─┴──┐     ┌─┴──┐   │        │    │
        │   │   │4.7k│*    │4.7k│*  │   USB- │ USB+
        │   │   └─┬──┘     └─┬──┘   │        │    │
        │   │     │          │       │        │    │
        │   │   ┌─┴──────────┴───┐   │     ┌──┴────┴───┐
        │   │   │                │   │     │  USB-C    │
        │   │   │   OLED I2C     │   │     │ Connector │
        │   │   │   128x32       │   │     │           │
        │   │   │   SSD1306      │   │     └───────────┘
        │   │   │                │   │
        │   │   │ VCC GND SCL SDA│   │
        │   │   └──┬───┬───┬───┬─┘   │
        │   │      │   │   │   │     │
       ┌┴───┴──────┴───┴───┘   └─────┘
       │
      GND

* Optional - most OLED modules and ESP32 have internal pull-ups
```

### Component Values

- **C1**: 10µF - Power supply decoupling for ESP32-C3
- **C2** (optional): 100nF - Additional high-frequency filtering
- **R1, R2** (optional): 4.7kΩ - I2C pull-up resistors

## KiCad Design Steps

### 1. Install KiCad

```bash
# Ubuntu/Debian
sudo apt install kicad

# Or latest version via Flatpak
flatpak install flathub org.kicad.KiCad
```

### 2. Open Project

```bash
cd ~/Projects/linux4lcd-esp-c3/hardware
kicad esp32c3-oled.kicad_pro
```

### 3. Add Component Libraries

You'll need these libraries:
- **ESP32-C3**: [ESP32 KiCad Library](https://github.com/espressif/kicad-libraries)
- **SSD1306 OLED**: [Adafruit KiCad Library](https://github.com/adafruit/Adafruit-KiCad-Library)

**Install libraries:**
```bash
# Clone libraries
cd ~/Documents/KiCad/libraries
git clone https://github.com/espressif/kicad-libraries.git
git clone https://github.com/adafruit/Adafruit-KiCad-Library.git

# In KiCad: Preferences → Manage Symbol Libraries → Add
```

### 4. Schematic Design Checklist

- [ ] Place ESP32-C3-WROOM-02 module
- [ ] Add OLED connector (4-pin header)
- [ ] Add decoupling capacitors
- [ ] Connect I2C lines (GPIO8 → SDA, GPIO9 → SCL)
- [ ] Add power connections (3.3V, GND)
- [ ] Add USB-C connector for programming
- [ ] Label all nets
- [ ] Add mounting holes
- [ ] Run ERC (Electrical Rule Check)

### 5. PCB Layout Guidelines

**Board Dimensions:**
- Minimum: 30x40mm (compact)
- Recommended: 40x60mm (easier assembly)
- With OLED: 30x70mm (vertical layout)

**Layer Stack:**
- Top: Signal + components
- Bottom: Ground plane + signal

**Track Widths:**
- Power (3.3V): 0.5mm minimum
- I2C signals: 0.25mm
- USB D+/D-: 0.25mm (impedance controlled if possible)

**Clearances:**
- General: 0.2mm minimum
- Between ESP32 antenna and ground: 2mm
- High-speed signals: 0.3mm

### 6. PCB Layout Steps

1. **Place components:**
   - ESP32-C3 in center
   - USB connector on one edge
   - OLED connector on top edge
   - Capacitors near ESP32 power pins

2. **Route power:**
   - Wide tracks for 3.3V and GND
   - Flood fill bottom layer with GND plane

3. **Route I2C:**
   - Keep SDA and SCL traces short
   - Route parallel if possible
   - Avoid crossing high-speed signals

4. **Route USB:**
   - Keep D+/D- traces equal length
   - Keep them close together
   - Avoid sharp angles

5. **Add mounting holes:**
   - 4x M3 holes in corners
   - 3mm hole, 6mm pad

6. **Silkscreen:**
   - Add pin labels
   - Add project name/version
   - Mark pin 1 on connectors

7. **Run DRC (Design Rule Check)**

## PCB Fabrication

### Export Gerber Files

In KiCad PCB Editor:
1. File → Fabrication Outputs → Gerbers (.gbr)
2. File → Fabrication Outputs → Drill Files (.drl)
3. Zip all files together

### Recommended PCB Manufacturers

**JLCPCB** ⭐ Best Value
- **Price**: $2 for 5 PCBs (100x100mm)
- **Quality**: Good
- **Speed**: 7-14 days with shipping
- **Assembly**: Available (SMT components)
- **Website**: https://jlcpcb.com/

**PCBWay**
- **Price**: $5 for 10 PCBs
- **Quality**: Very good
- **Speed**: 7-14 days
- **Website**: https://www.pcbway.com/

**OSH Park** (USA)
- **Price**: $1/sq inch for 3 PCBs
- **Quality**: Excellent (purple PCBs!)
- **Speed**: 10-14 days domestic
- **Website**: https://oshpark.com/

### PCB Specifications for Ordering

```
Board Size: 40mm x 60mm (or custom)
Layers: 2 (Top + Bottom)
Thickness: 1.6mm
Material: FR-4
Copper Weight: 1oz (35µm)
Surface Finish: HASL or ENIG
Solder Mask Color: Green (or any)
Silkscreen: White
Min Track/Space: 6/6 mil (0.15mm)
Min Hole Size: 0.3mm
```

## Assembly Instructions

### DIY Assembly

**Tools Needed:**
- Soldering iron (temperature controlled)
- Solder (63/37 or lead-free)
- Tweezers
- Flux
- Multimeter
- Magnifying glass/microscope

**Assembly Order:**
1. Solder ESP32-C3 module (hot air or reflow)
2. Solder small components (capacitors, resistors)
3. Solder connectors (USB, OLED header)
4. Inspect all joints
5. Clean flux residue
6. Test continuity with multimeter

**SMT Tips:**
- Use solder paste for ESP32 module
- Reflow at 245°C peak (lead-free profile)
- Or use hot air at 350°C
- Check for bridges under microscope

### JLCPCB Assembly Service

JLCPCB can assemble your board:

1. **Upload Gerber + BOM + CPL files**
2. **Select components from LCSC**
3. **Review assembly positions**
4. **Order:** ~$8-20 extra for assembly

**Components available at JLCPCB:**
- ESP32-C3-WROOM-02 ✓
- 0805 capacitors ✓
- 0805 resistors ✓
- USB-C connectors ✓

**You solder:**
- OLED header (through-hole)
- OLED display

## Design Variations

### Vertical Layout (Display on Top)
```
┌──────────────┐
│  OLED 128x32 │
│  ┌────────┐  │
│  │Display │  │
│  └────────┘  │
└──────────────┘
┌──────────────┐
│   PCB Board  │
│              │
│   ESP32-C3   │
│              │
│   USB-C ○    │
└──────────────┘
```

### Horizontal Layout (Side by Side)
```
┌─────────┬──────────────┐
│  OLED   │   PCB Board  │
│         │              │
│ ┌────┐  │  ESP32-C3    │
│ │Disp│  │              │
│ └────┘  │  USB-C ○     │
└─────────┴──────────────┘
```

### Compact Layout (Ultra-small)
```
┌────────┐
│ OLED   │
│ ┌────┐ │
│ │Disp│ │
│ └────┘ │
│ESP32-C3│
│ USB-C○ │
└────────┘
Size: 30x50mm
```

## Testing PCB

### Visual Inspection
- [ ] Check for solder bridges
- [ ] Verify component orientation
- [ ] Check for cold solder joints
- [ ] Inspect under microscope

### Electrical Testing
1. **Power test:**
   ```bash
   # Check 3.3V rail with multimeter
   # Should be 3.25-3.35V
   ```

2. **Continuity test:**
   ```bash
   # Verify I2C connections
   # ESP32 GPIO8 → OLED SDA
   # ESP32 GPIO9 → OLED SCL
   ```

3. **USB test:**
   ```bash
   # Connect USB cable
   # Should enumerate as USB device
   dmesg | grep ttyACM
   ```

4. **OLED test:**
   ```bash
   # Upload Arduino sketch
   # Should display "LCD4Linux Ready"
   ```

## Python Script for PCB Verification

```python
#!/usr/bin/env python3
"""PCB verification script"""

import serial
import time

def test_board(port='/dev/ttyACM0'):
    print("Testing ESP32-C3 OLED PCB...")
    
    try:
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)
        
        # Test PING
        ser.write(b"PING\n")
        response = ser.readline().decode().strip()
        
        if "PONG" in response:
            print("✓ Board communication OK")
        else:
            print("✗ Communication failed")
            return False
        
        # Test INFO
        ser.write(b"INFO\n")
        response = ser.readline().decode().strip()
        print(f"✓ Display info: {response}")
        
        # Test CLEAR
        ser.write(b"CLEAR\n")
        ser.readline()
        
        # Test TEXT
        ser.write(b"TEXT:0:0:1:PCB Test OK\n")
        ser.readline()
        
        # Update display
        ser.write(b"DISPLAY\n")
        response = ser.readline().decode().strip()
        
        if "OK" in response:
            print("✓ Display working correctly")
            print("\n✓ PCB verification complete!")
            return True
        
        ser.close()
        
    except Exception as e:
        print(f"✗ Error: {e}")
        return False
    
    return False

if __name__ == '__main__':
    test_board()
```

## Troubleshooting

### Common Issues

**USB not recognized:**
- Check USB D+/D- connections
- Verify USB-C connector orientation
- Test with different cable
- Hold BOOT button while connecting

**OLED not working:**
- Verify I2C address (0x3C or 0x3D)
- Check SDA/SCL connections
- Test with I2C scanner
- Verify 3.3V is present at display

**ESP32 not programming:**
- Check EN (reset) and IO9 (boot) pins
- Verify 3.3V power supply
- Use external USB-UART adapter
- Check for shorts on power rails

**Display garbled:**
- Check I2C clock speed (reduce if needed)
- Verify ground connection
- Check for noise on I2C lines
- Add pull-up resistors if missing

## Resources

- **KiCad Documentation**: https://docs.kicad.org/
- **ESP32-C3 Datasheet**: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf
- **SSD1306 Datasheet**: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
- **PCB Design Tutorial**: https://www.youtube.com/watch?v=35YuILUlfGs
- **JLCPCB Tutorial**: https://www.youtube.com/watch?v=C7-8nUU6e3E

## Next Steps

1. ✓ Review this guide
2. ⚡ Design schematic in KiCad
3. ⚡ Layout PCB
4. ⚡ Export Gerber files
5. ⚡ Order from JLCPCB
6. ⚡ Receive and assemble
7. ⚡ Test with verification script
8. ⚡ Deploy with LCD4Linux!

Need help with KiCad design? Ask for specific guidance!
