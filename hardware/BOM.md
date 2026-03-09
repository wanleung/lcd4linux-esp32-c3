# Bill of Materials (BOM) - ESP32-C3 OLED Display

## Required Components

### Main Components

| Ref | Qty | Component | Description | Part Number | Approx Price | Where to Buy |
|-----|-----|-----------|-------------|-------------|--------------|--------------|
| U1 | 1 | ESP32-C3-WROOM-02 | WiFi/BLE Module | ESP32-C3-WROOM-02 | $2.50 | [DigiKey](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-WROOM-02/13930819), [Mouser](https://www.mouser.com/ProductDetail/Espressif-Systems/ESP32-C3-WROOM-02), [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32-C3-WROOM-02) |
| DISP1 | 1 | OLED 128x32 I2C | SSD1306 OLED Display | 128x32 0.91" | $3-5 | [Amazon](https://www.amazon.com/s?k=128x32+oled+i2c), [AliExpress](https://www.aliexpress.com/wholesale?SearchText=128x32+oled+i2c+ssd1306), [Adafruit #4440](https://www.adafruit.com/product/4440) |

**Alternative: Pre-assembled Dev Boards**
| Ref | Qty | Component | Description | Part Number | Approx Price | Where to Buy |
|-----|-----|-----------|-------------|-------------|--------------|--------------|
| - | 1 | ESP32-C3 DevKit | Development Board with USB | ESP32-C3-DevKitM-1 | $5-8 | [Amazon](https://www.amazon.com/s?k=ESP32-C3+DevKit), [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32-C3+development+board), [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) |

### Recommended ESP32-C3 Development Boards

**Option 1: Seeed XIAO ESP32C3** ⭐ Recommended
- **Price:** $4.99
- **Size:** 21x17.5mm (tiny!)
- **Features:** USB-C, battery charging, castellated pads
- **Buy:** [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html), [Amazon](https://www.amazon.com/dp/B0B94J5N6T)
- **Why:** Small form factor, great documentation, USB-C

**Option 2: ESP32-C3-DevKitM-1**
- **Price:** $5-7
- **Size:** Standard dev board
- **Features:** Micro USB, breadboard friendly
- **Buy:** [Espressif Official](https://www.espressif.com/en/products/devkits/esp32-c3-devkitm-1), [Amazon](https://www.amazon.com/s?k=ESP32-C3-DevKitM-1)
- **Why:** Official Espressif board, good support

**Option 3: Generic ESP32-C3 Mini**
- **Price:** $2-4
- **Size:** Varies
- **Features:** Basic USB
- **Buy:** [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32-C3+mini+board)
- **Why:** Cheapest option

### OLED Display Options

**Option 1: Adafruit 128x32 OLED** ⭐ Best Quality
- **Part:** Adafruit #4440
- **Price:** $14.95
- **Interface:** I2C (0x3C address)
- **Buy:** [Adafruit](https://www.adafruit.com/product/4440)
- **Why:** High quality, great support, includes header

**Option 2: Generic 128x32 OLED** ⭐ Best Value
- **Part:** Generic SSD1306
- **Price:** $3-5
- **Interface:** I2C (0x3C or 0x3D)
- **Buy:** [Amazon](https://www.amazon.com/s?k=128x32+oled+i2c), [AliExpress](https://www.aliexpress.com/wholesale?SearchText=0.91+inch+oled+128x32)
- **Why:** Cheap, widely available

**Option 3: Waveshare 128x32 OLED**
- **Part:** Waveshare 16004
- **Price:** $8-10
- **Interface:** I2C/SPI
- **Buy:** [Waveshare](https://www.waveshare.com/product/displays/oled/oled-0.91-128x32.htm), [Amazon](https://www.amazon.com/s?k=waveshare+128x32+oled)
- **Why:** Good quality, flexible interface

### Connectors & Cables

| Ref | Qty | Component | Description | Price | Where to Buy |
|-----|-----|-----------|-------------|-------|--------------|
| J1 | 1 | 4-pin female header | 2.54mm pitch | $0.20 | Any electronics supplier |
| - | 1 | USB-C cable | For ESP32-C3 | $2-5 | Any electronics store |
| - | 4 | Jumper wires | Female-female | $1 | [Amazon](https://www.amazon.com/s?k=jumper+wires) |

### Optional Components for Custom PCB

| Ref | Qty | Component | Value | Package | Description | Price | Where to Buy |
|-----|-----|-----------|-------|---------|-------------|-------|--------------|
| C1 | 1 | Capacitor | 10µF | 0805 | Decoupling cap | $0.10 | [DigiKey](https://www.digikey.com/), [LCSC](https://www.lcsc.com/) |
| C2 | 1 | Capacitor | 100nF | 0805 | Decoupling cap | $0.05 | DigiKey, LCSC |
| R1 | 2 | Resistor | 4.7kΩ | 0805 | I2C pull-up | $0.05 | DigiKey, LCSC |
| J1 | 1 | USB-C connector | - | SMD | USB connector | $0.50 | [LCSC](https://www.lcsc.com/search?q=usb-c+connector), [JLCPCB](https://jlcpcb.com/) |
| J2 | 1 | 4-pin header | 2.54mm | TH | OLED connector | $0.10 | Any supplier |

## Complete Kits

### Pre-wired Kit (Easiest) ⭐⭐⭐
**Total Cost:** ~$10-15

Buy these together:
1. ESP32-C3 DevKit board ($5-8)
2. 128x32 OLED with pins ($3-5)
3. 4x female-female jumper wires ($1)

**Assembly:** Just connect 4 wires!

### Custom PCB Kit (Advanced)
**Total Cost:** ~$20-30 for 5 boards

1. ESP32-C3-WROOM-02 module ($2.50)
2. OLED display ($3-5)
3. Components listed above (~$2)
4. PCB fabrication from JLCPCB (~$5 for 5 boards)
5. Assembly (DIY or JLCPCB service)

## Supplier Links

### Recommended Suppliers

**USA/International:**
- **Amazon** - Fast shipping, easy returns
- **DigiKey** - Huge selection, trusted supplier
- **Mouser** - Professional components
- **Adafruit** - Quality products, great support
- **SparkFun** - Good tutorials and docs

**Budget Options:**
- **AliExpress** - Cheapest, 2-4 week shipping
- **Banggood** - Similar to AliExpress
- **eBay** - Mixed sellers, check ratings

**PCB Fabrication:**
- **JLCPCB** ⭐ - $2 for 5 PCBs, assembly service
- **PCBWay** - Good quality, slightly more expensive
- **OSH Park** - USA-based, purple PCBs
- **ALLPCB** - Budget option

**Component Sources (Bulk):**
- **LCSC** ⭐ - Huge stock, works with JLCPCB
- **Arrow** - Good prices, authorized distributor
- **Newark/Farnell** - Professional supplier

## Cost Breakdown

### Option A: Development Board Setup (Recommended for Beginners)
| Item | Price |
|------|-------|
| ESP32-C3 DevKit | $5-8 |
| 128x32 OLED Display | $3-5 |
| Jumper wires (set) | $1-2 |
| USB Cable | $2-3 |
| **Total** | **$11-18** |

### Option B: Custom PCB (Advanced)
| Item | Price |
|------|-------|
| ESP32-C3 module | $2.50 |
| OLED Display | $3-5 |
| PCB (5 pcs from JLCPCB) | $5 |
| Components | $2-3 |
| Shipping | $5-10 |
| **Total** | **$17.50-25.50** |

## Quick Shopping List

**For immediate start (copy/paste to cart):**

1. **Search on Amazon:**
   - "ESP32-C3 development board"
   - "128x32 OLED I2C"
   - "Female to Female jumper wires"

2. **Or buy from AliExpress (cheaper, slower):**
   - "ESP32-C3 DevKit"
   - "0.91 inch OLED 128x32 I2C SSD1306"
   - "Dupont jumper wires female"

## Verification Checklist

Before ordering, verify:
- [ ] ESP32-C3 has USB support (not all variants do)
- [ ] OLED is 128x32 pixels (not 128x64)
- [ ] OLED interface is I2C (not SPI)
- [ ] OLED address is 0x3C or 0x3D
- [ ] Display has 4 pins: VCC, GND, SDA, SCL

## Shipping and Lead Times

| Supplier | Shipping Time | Shipping Cost |
|----------|---------------|---------------|
| Amazon (Prime) | 1-2 days | Free |
| DigiKey (USA) | 2-3 days | $5-10 |
| Adafruit (USA) | 3-5 days | $4-8 |
| AliExpress | 15-30 days | Free |
| JLCPCB | 7-14 days | $5-20 |

## Notes

- Prices are approximate and in USD (March 2026)
- Bulk purchases reduce per-unit cost
- Check for coupons on AliExpress (often 10-20% off)
- JLCPCB often has $2 PCB deals for new users
- Consider buying extra components for prototyping

## Next Steps

1. **Order components** (use quick shopping list above)
2. **While waiting**, upload Arduino firmware
3. **On arrival**, connect per wiring diagram
4. **Test** with test-connection.sh script
5. **Deploy** Python bridge or native driver

Need help ordering? Ask for specific supplier recommendations!
