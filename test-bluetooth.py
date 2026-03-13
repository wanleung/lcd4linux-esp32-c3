#!/usr/bin/env python3
"""
Quick Bluetooth connection test for ESP32-C3 display

This script quickly tests if you can connect to your ESP32 via Bluetooth.

Copyright (C) 2024-2026 Wan Leung Wong
Licensed under GNU GPL v3 or later
"""

import asyncio
import sys

try:
    from bleak import BleakScanner
except ImportError:
    print("Error: 'bleak' library not installed")
    print("Install with: pip3 install bleak")
    sys.exit(1)


async def scan_devices(timeout=10.0):
    """Scan for BLE devices and look for LCD4Linux ESP32"""
    print(f"Scanning for Bluetooth devices (timeout: {timeout}s)...")
    print("Looking for: LCD4Linux-ESP32\n")
    
    devices = await BleakScanner.discover(timeout=timeout)
    
    lcd4linux_devices = []
    other_devices = []
    
    for device in devices:
        if device.name and 'LCD4Linux' in device.name:
            lcd4linux_devices.append(device)
        else:
            other_devices.append(device)
    
    # Print LCD4Linux devices first
    if lcd4linux_devices:
        print("✅ Found LCD4Linux devices:")
        for device in lcd4linux_devices:
            print(f"   • {device.name}")
            print(f"     Address: {device.address}")
            print(f"     RSSI: {device.rssi} dBm")
            print()
    else:
        print("❌ No LCD4Linux devices found!")
        print()
    
    # Print other devices
    if other_devices:
        print(f"Other BLE devices found ({len(other_devices)}):")
        for device in other_devices[:10]:  # Show first 10
            name = device.name or "(Unknown)"
            print(f"   • {name} - {device.address}")
        
        if len(other_devices) > 10:
            print(f"   ... and {len(other_devices) - 10} more devices")
    
    print("\n" + "="*60)
    
    if lcd4linux_devices:
        print("✅ SUCCESS! Your ESP32 is advertising via Bluetooth")
        print("\nNext steps:")
        print("1. Run the Bluetooth daemon:")
        print("   ./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode test")
        print("\n2. Or use it in system info mode:")
        print("   ./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode system")
    else:
        print("❌ ESP32 not found. Troubleshooting:")
        print("\n1. Check ESP32 is powered on")
        print("2. Verify firmware is uploaded (should show 'Serial + BLE' on display)")
        print("3. Check Bluetooth is enabled:")
        print("   sudo systemctl status bluetooth")
        print("4. Try scanning again with longer timeout:")
        print("   python3 test-bluetooth.py")
        print("\n5. If you changed the device name in config.h, search for it above")


async def main():
    try:
        await scan_devices(timeout=10.0)
    except Exception as e:
        print(f"Error during scan: {e}")
        print("\nTroubleshooting:")
        print("- Ensure Bluetooth is enabled: sudo systemctl start bluetooth")
        print("- Check permissions: sudo usermod -a -G bluetooth $USER")
        print("- Try running with sudo: sudo python3 test-bluetooth.py")


if __name__ == '__main__':
    print("="*60)
    print("LCD4Linux ESP32 Bluetooth Test")
    print("="*60 + "\n")
    
    asyncio.run(main())
