#!/usr/bin/env python3
"""
LCD4Linux Bluetooth Daemon

This daemon connects to the ESP32-C3 OLED driver via Bluetooth BLE
and provides the same functionality as the USB Serial bridge.

It can be used as a systemd service to run continuously or in different modes.

Usage:
    ./lcd4linux-bt-daemon.py --name LCD4Linux-ESP32 --mode system
    ./lcd4linux-bt-daemon.py --address AA:BB:CC:DD:EE:FF --mode custom
    
Modes:
    - system: Display system stats (CPU, memory, time) - updates every 2 seconds
    - custom: Read from stdin and display
    - test: Run test pattern once
    - daemon: Run as background daemon (system mode)
"""

import asyncio
import sys
import argparse
import datetime
import psutil
import time
from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError

# Nordic UART Service UUIDs
UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # ESP32 receives on this
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # ESP32 sends on this


class ESP32BLEDisplay:
    """Bluetooth BLE interface for ESP32-C3 display"""
    
    def __init__(self, device_address=None, device_name=None):
        """
        Initialize BLE connection
        
        Args:
            device_address: MAC address of the ESP32 device (e.g., 'AA:BB:CC:DD:EE:FF')
            device_name: Name of the ESP32 device (e.g., 'LCD4Linux-ESP32')
        """
        self.device_address = device_address
        self.device_name = device_name
        self.client = None
        self.response_queue = asyncio.Queue()
        
    async def find_device(self, timeout=10.0):
        """Scan for the ESP32 device by name or address"""
        print(f"Scanning for BLE devices (timeout: {timeout}s)...")
        
        devices = await BleakScanner.discover(timeout=timeout)
        
        for device in devices:
            if self.device_address and device.address.upper() == self.device_address.upper():
                print(f"Found device by address: {device.name} ({device.address})")
                return device.address
            elif self.device_name and device.name and self.device_name in device.name:
                print(f"Found device by name: {device.name} ({device.address})")
                return device.address
        
        return None
    
    async def connect(self):
        """Connect to the ESP32 device"""
        if not self.device_address:
            self.device_address = await self.find_device()
            
        if not self.device_address:
            raise Exception(f"Could not find device: {self.device_name or 'Unknown'}")
        
        print(f"Connecting to {self.device_address}...")
        self.client = BleakClient(self.device_address)
        await self.client.connect()
        
        if not self.client.is_connected:
            raise Exception("Failed to connect to device")
        
        print("Connected successfully!")
        
        # Setup notification handler for receiving data from ESP32
        await self.client.start_notify(UART_TX_CHAR_UUID, self._notification_handler)
        
        # Wait a bit for ESP32 to send ready signal
        await asyncio.sleep(1)
        
        return True
    
    def _notification_handler(self, sender, data):
        """Handle notifications from ESP32 (responses)"""
        response = data.decode('utf-8').strip()
        print(f"<< {response}")
        self.response_queue.put_nowait(response)
    
    async def send_command(self, cmd, wait_response=True):
        """Send a command to ESP32 and optionally wait for response"""
        if not self.client or not self.client.is_connected:
            raise Exception("Not connected to device")
        
        # Add newline if not present
        if not cmd.endswith('\n'):
            cmd += '\n'
        
        print(f">> {cmd.strip()}")
        
        if not wait_response:
            # Send without waiting
            await self.client.write_gatt_char(UART_RX_CHAR_UUID, cmd.encode('utf-8'))
            return "OK"
        
        # Clear queue before sending
        while not self.response_queue.empty():
            self.response_queue.get_nowait()
        
        # Send command
        await self.client.write_gatt_char(UART_RX_CHAR_UUID, cmd.encode('utf-8'))
        
        # Wait for response
        try:
            response = await asyncio.wait_for(self.response_queue.get(), timeout=2.0)
            return response
        except asyncio.TimeoutError:
            return "TIMEOUT"
    
    async def clear(self):
        """Clear the display (no wait - faster)"""
        return await self.send_command("CLEAR", wait_response=False)
    
    async def text(self, x, y, size, text):
        """Draw text at position (x,y) with specified size (no wait - faster)"""
        # Escape colons in text
        text = text.replace(':', '\\:')
        return await self.send_command(f"TEXT:{x}:{y}:{size}:{text}", wait_response=False)
    
    async def display(self):
        """Update the physical display (wait for completion)"""
        return await self.send_command("DISPLAY", wait_response=True)
    
    async def ping(self):
        """Check if device is alive"""
        return await self.send_command("PING")
    
    async def get_info(self):
        """Get display info"""
        return await self.send_command("INFO")
    
    async def line(self, x0, y0, x1, y1):
        """Draw a line"""
        return await self.send_command(f"LINE:{x0}:{y0}:{x1}:{y1}")
    
    async def rect(self, x, y, w, h, fill=0, color=1):
        """Draw a rectangle (color: 0=black, 1=white)"""
        return await self.send_command(f"RECT:{x}:{y}:{w}:{h}:{fill}:{color}", wait_response=False)
    
    async def clear_area(self, x, y, w, h):
        """Clear a specific area by drawing a black filled rectangle"""
        return await self.rect(x, y, w, h, fill=1, color=0)
    
    async def circle(self, x, y, r, fill=0):
        """Draw a circle"""
        return await self.send_command(f"CIRCLE:{x}:{y}:{r}:{fill}")
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("Disconnected from device")


async def draw_system_info(display):
    """Draw system information on display (no full clear - just overwrite)"""
    # Clear only the text areas with black rectangles (prevents full screen flash)
    await display.clear_area(0, 0, 128, 8)   # Clear line 1 area
    await display.clear_area(0, 10, 128, 8)  # Clear line 2 area
    await display.clear_area(0, 20, 128, 12) # Clear line 3 area
    
    # Line 1: Current time
    current_time = datetime.datetime.now().strftime('%H:%M:%S')
    await display.text(0, 0, 1, f"Time: {current_time}")
    
    # Line 2: CPU usage
    cpu_percent = psutil.cpu_percent(interval=0.1)
    await display.text(0, 10, 1, f"CPU: {cpu_percent:.0f}%")
    
    # Line 3: Memory usage
    mem = psutil.virtual_memory()
    mem_mb = mem.used / 1024 / 1024
    await display.text(0, 20, 1, f"Mem: {mem_mb:.0f}MB")
    
    await display.display()


async def draw_custom_text(display, lines):
    """Draw custom text lines"""
    await display.clear()
    
    y_positions = [0, 10, 20]
    for i, line in enumerate(lines[:3]):  # Max 3 lines for 32px height
        await display.text(0, y_positions[i], 1, line)
    
    await display.display()


async def test_pattern(display):
    """Display a test pattern"""
    await display.clear()
    
    # Text
    await display.text(0, 0, 1, "LCD4Linux Ready!")
    await display.text(0, 10, 1, "ESP32-C3 BLE")
    
    # Line
    await display.line(0, 22, 127, 22)
    
    # Box
    await display.rect(0, 24, 40, 7, 0)
    
    await display.display()


async def run_system_mode(display, interval=2.5):
    """Run in system info mode"""
    print(f"Starting system info mode (update every {interval}s)...")
    print("Press Ctrl+C to stop")
    
    try:
        while True:
            await draw_system_info(display)
            await asyncio.sleep(interval)
    except KeyboardInterrupt:
        print("\nStopping...")
    except Exception as e:
        print(f"Error in system mode: {e}")


async def run_custom_mode(display):
    """Run in custom mode - read from stdin"""
    print("Custom mode: Enter up to 3 lines of text (Ctrl+D to send, Ctrl+C to quit):")
    
    try:
        while True:
            print("\nEnter lines:")
            lines = []
            try:
                for i in range(3):
                    line = input(f"Line {i+1}: ")
                    if line:
                        lines.append(line)
            except EOFError:
                pass
            
            if lines:
                await draw_custom_text(display, lines)
            
    except KeyboardInterrupt:
        print("\nStopping...")


async def run_test_mode(display):
    """Run test pattern once"""
    print("Running test pattern...")
    await test_pattern(display)
    print("Test complete!")


async def main():
    parser = argparse.ArgumentParser(description='LCD4Linux Bluetooth BLE Daemon')
    parser.add_argument('--address', help='ESP32 MAC address (e.g., AA:BB:CC:DD:EE:FF)')
    parser.add_argument('--name', default='LCD4Linux-ESP32', help='ESP32 device name (default: LCD4Linux-ESP32)')
    parser.add_argument('--mode', choices=['system', 'custom', 'test', 'daemon'], 
                       default='system', help='Operation mode (default: system)')
    parser.add_argument('--interval', type=float, default=2.5, 
                       help='Update interval in seconds for system mode (default: 2.5)')
    parser.add_argument('--scan-timeout', type=float, default=10.0, 
                       help='BLE scan timeout in seconds (default: 10.0)')
    
    args = parser.parse_args()
    
    # Create display interface
    display = ESP32BLEDisplay(device_address=args.address, device_name=args.name)
    
    try:
        # Connect to device
        await display.connect()
        
        # Get device info
        info = await display.get_info()
        print(f"Device info: {info}")
        
        # Run selected mode
        if args.mode == 'system' or args.mode == 'daemon':
            await run_system_mode(display, interval=args.interval)
        elif args.mode == 'custom':
            await run_custom_mode(display)
        elif args.mode == 'test':
            await run_test_mode(display)
            
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
    finally:
        await display.disconnect()


if __name__ == '__main__':
    # Run the async main function
    asyncio.run(main())
