# Screen Anti-Flicker Guide

This guide explains the screen flicker issue and how the fixes work.

## The Problem

When updating OLED displays frequently (like showing real-time system info), the screen can blink or flicker. This happens because:

1. **Full screen clears** - Clearing the entire display turns all pixels off
2. **Frequent updates** - Updating every 1-2 seconds makes flicker more visible  
3. **Buffering delays** - Time between clearing and redrawing causes visible black frames

## The Solution

Three techniques were implemented to eliminate flicker:

### 1. Partial Screen Clears (New Feature!)

Instead of clearing the entire display, we now clear only the text areas using **black-filled rectangles**.

**New ESP32 Command Format:**
```
RECT:x:y:w:h:fill:color
```
- `color` parameter: `0` = black, `1` = white (default)
- Example: `RECT:0:0:128:8:1:0` - filled black rectangle to clear text line

**Python API:**
```python
# Old way (causes flicker)
display.clear()
display.text(0, 0, 1, "Hello")
display.display()

# New way (smooth update)
display.clear_rect(0, 0, 128, 8)  # Clear only text area
display.text(0, 0, 1, "Hello")
display.display()
```

### 2. Optimized Update Intervals

Default update intervals increased to reduce flicker visibility:

- **USB Serial** (`lcd4linux-bridge.py`): 2.5 seconds (was 1.0)
- **Bluetooth** (`lcd4linux-bt-daemon.py`): 2.5 seconds (was 2.0)

**Customize the interval:**
```bash
# USB Serial - update every 5 seconds
./lcd4linux-bridge.py --mode system --interval 5.0

# Bluetooth - update every 3 seconds
./lcd4linux-bt-daemon.py --mode system --interval 3.0
```

### 3. Backwards Compatible

The RECT command still works without the color parameter:
```
RECT:0:0:50:20:1      # White filled rectangle (old format)
RECT:0:0:50:20:1:1    # White filled rectangle (new format)
RECT:0:0:50:20:1:0    # Black filled rectangle (new feature)
RECT:0:0:50:20:0:1    # White outline (new format)
```

## How to Apply the Fix

### Option 1: Update Firmware Only (Recommended)

If you just want to reduce flicker with the default settings:

1. **Upload new firmware:**
   - Open [lcd4linux_esp32.ino](lcd4linux_esp32.ino) in Arduino IDE
   - Upload to your ESP32-C3

2. **The Python scripts will automatically use the new anti-flicker method** ✓

### Option 2: Adjust Update Speed Only

If you don't want to re-upload firmware:

```bash
# Slow down updates (less flicker but less frequent updates)
./lcd4linux-bridge.py --mode system --interval 5.0

# Or for Bluetooth
./lcd4linux-bt-daemon.py --mode system --interval 5.0
```

### Option 3: Full Update (Best Results)

For the smoothest display:

1. **Upload new firmware** (see Option 1)
2. **Use optimized scripts** (already updated)
3. **Customize interval** if needed

## Testing the Fix

### Test 1: System Info Mode (USB Serial)
```bash
./lcd4linux-bridge.py --port /dev/ttyACM0 --mode system --interval 2.5
```

Watch the display - it should update smoothly without full-screen flicker.

### Test 2: System Info Mode (Bluetooth)
```bash
./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode system --interval 2.5
```

The display should update smoothly every 2.5 seconds.

### Test 3: Verify Partial Updates

Run this test script to see partial clears in action:

```python
#!/usr/bin/env python3
from lcd4linux_bridge import ESP32Display
import time

display = ESP32Display('/dev/ttyACM0')

# Draw initial content
display.clear()
display.text(0, 0, 1, "Line 1: Original")
display.text(0, 10, 1, "Line 2: Original")
display.text(0, 20, 1, "Line 3: Original")
display.display()
time.sleep(2)

# Update only line 2 (watch for flicker)
print("Updating line 2 only...")
display.clear_rect(0, 10, 128, 8)  # Clear only line 2
display.text(0, 10, 1, "Line 2: UPDATED!")
display.display()

print("Done! Line 2 updated with minimal flicker.")
```

## Understanding the Update Cycle

**Old method (full screen clear):**
```
Clear entire screen → Black screen visible → Draw new content → Display
                       ^^^^^^^^^^^^^^
                       Flicker happens here!
```

**New method (partial clear):**
```
Clear text area 1 → Draw text 1 → Clear text area 2 → Draw text 2 → Display
(Only affected pixels turn black briefly - much less noticeable)
```

## Troubleshooting

### Still seeing flicker?

1. **Increase update interval:**
   ```bash
   --interval 5.0  # or even 10.0
   ```

2. **Check if old firmware:**
   - The new firmware should show "Serial + BLE" on startup
   - Upload the latest [lcd4linux_esp32.ino](lcd4linux_esp32.ino)

3. **Display hardware limits:**
   - Some OLED displays have slower refresh rates
   - Try reducing I2C speed in [config.h](config.h) if flicker persists

### Getting errors about clear_rect?

Make sure you're using the updated scripts:
```bash
# Check for the new method
grep "clear_rect" lcd4linux-bridge.py
```

Should show: `def clear_rect(self, x, y, w, h):`

## Advanced: Custom Anti-Flicker Code

Use partial clears in your own scripts:

```python
import time
import datetime
from lcd4linux_bridge import ESP32Display

display = ESP32Display('/dev/ttyACM0')

# Initial draw
display.clear()
display.text(0, 0, 1, "Custom Display")
display.line(0, 9, 127, 9)
display.display()

# Update loop with minimal flicker
while True:
    # Only clear and update the time area
    display.clear_rect(0, 11, 128, 8)
    
    current_time = datetime.datetime.now().strftime('%H:%M:%S')
    display.text(0, 11, 1, f"Time: {current_time}")
    
    display.display()
    time.sleep(1)
```

## Performance Comparison

| Method | Flicker Level | Update Speed | Complexity |
|--------|---------------|--------------|------------|
| Full clear (old) | High | Fast | Simple |
| Partial clear (new) | Low | Fast | Moderate |
| Slower updates | Medium | Slow | Simple |
| Partial + Slow (recommended) | Very Low | Moderate | Moderate |

## Summary

✅ **Firmware updated** - Supports black-filled rectangles for partial clears  
✅ **Python scripts optimized** - Use `clear_rect()` instead of full `clear()`  
✅ **Update intervals tuned** - Default 2.5s for smooth operation  
✅ **Backwards compatible** - Old commands still work  
✅ **Customizable** - Adjust `--interval` to your preference  

**Result: Smooth, flicker-free display updates! 🎉**

## Related Files

- [lcd4linux_esp32.ino](lcd4linux_esp32.ino) - ESP32 firmware with anti-flicker support
- [lcd4linux-bridge.py](lcd4linux-bridge.py) - USB Serial daemon with optimized updates
- [lcd4linux-bt-daemon.py](lcd4linux-bt-daemon.py) - Bluetooth daemon with optimized updates
- [config.h](config.h) - Hardware configuration
