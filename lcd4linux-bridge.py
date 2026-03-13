#!/usr/bin/env python3
"""
LCD4Linux to ESP32-C3 Bridge Script

This script acts as a bridge between lcd4linux daemon and the ESP32-C3 OLED driver.
It can either:
1. Listen to a simple text-based input and send it to the display
2. Be called periodically to update system information

Usage:
    ./lcd4linux-bridge.py --port /dev/ttyACM0 --mode system
    
Modes:
    - system: Display system stats (CPU, memory, time)
    - custom: Read from stdin and display
    - test: Run test pattern

Copyright (C) 2024-2026 Wan Leung Wong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

import serial
import time
import sys
import argparse
import datetime
import psutil
import os

class ESP32Display:
    def __init__(self, port='/dev/ttyACM0', baudrate=115200, timeout=1):
        """Initialize serial connection to ESP32-C3"""
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)  # Wait for ESP32 to initialize
        
        # Wait for ready signal
        ready = False
        for _ in range(10):
            if self.ser.in_waiting:
                line = self.ser.readline().decode('utf-8').strip()
                if 'READY' in line:
                    ready = True
                    break
            time.sleep(0.5)
        
        if not ready:
            print("Warning: ESP32 not ready, continuing anyway...")
    
    def send_command(self, cmd, wait_response=True):
        """Send a command to ESP32 and optionally wait for response"""
        self.ser.write(f"{cmd}\n".encode('utf-8'))
        self.ser.flush()
        
        if not wait_response:
            return "OK"
        
        # Wait for response
        timeout = time.time() + 1.0
        while time.time() < timeout:
            if self.ser.in_waiting:
                response = self.ser.readline().decode('utf-8').strip()
                return response
            time.sleep(0.01)
        return "TIMEOUT"
    
    def clear(self):
        """Clear the display (no wait for response - faster)"""
        return self.send_command("CLEAR", wait_response=False)
    
    def text(self, x, y, size, text):
        """Draw text at position (x,y) with specified size (no wait - faster)"""
        # Escape colons in text
        text = text.replace(':', '\\:')
        return self.send_command(f"TEXT:{x}:{y}:{size}:{text}", wait_response=False)
    
    def display(self):
        """Update the physical display (wait for response to ensure completion)"""
        return self.send_command("DISPLAY", wait_response=True)
        """Update the physical display"""
        return self.send_command("DISPLAY")
    
    def ping(self):
        """Check if device is alive"""
        return self.send_command("PING")
    
    def get_info(self):
        """Get display info"""
        return self.send_command("INFO")
    
    def line(self, x0, y0, x1, y1):
        """Draw a line"""
        return self.send_command(f"LINE:{x0}:{y0}:{x1}:{y1}")
    
    def rect(self, x, y, w, h, fill=0, color=1):
        """Draw a rectangle (color: 0=black, 1=white)"""
        return self.send_command(f"RECT:{x}:{y}:{w}:{h}:{fill}:{color}", wait_response=False)
    
    def clear_area(self, x, y, w, h):
        """Clear a specific area by drawing a black filled rectangle"""
        return self.rect(x, y, w, h, fill=1, color=0)
    
    def circle(self, x, y, r, fill=0):
        """Draw a circle"""
        return self.send_command(f"CIRCLE:{x}:{y}:{r}:{fill}")
    
    def close(self):
        """Close serial connection"""
        self.ser.close()


def draw_system_info(display):
    """Draw system information on display (no full clear - just overwrite)"""
    # Clear only the text areas with black rectangles (prevents full screen flash)
    display.clear_area(0, 0, 128, 8)   # Clear line 1 area
    display.clear_area(0, 10, 128, 8)  # Clear line 2 area  
    display.clear_area(0, 20, 128, 12) # Clear line 3 area
    
    # Line 1: Current time
    current_time = datetime.datetime.now().strftime('%H:%M:%S')
    display.text(0, 0, 1, f"Time: {current_time}")
    
    # Line 2: CPU usage
    cpu_percent = psutil.cpu_percent(interval=0.1)
    display.text(0, 10, 1, f"CPU: {cpu_percent:.0f}%")
    
    # Line 3: Memory usage
    mem = psutil.virtual_memory()
    mem_mb = mem.used / 1024 / 1024
    display.text(0, 20, 1, f"Mem: {mem_mb:.0f}MB")
    
    display.display()


def draw_custom_text(display, lines):
    """Draw custom text lines"""
    display.clear()
    
    y_positions = [0, 10, 20]
    for i, line in enumerate(lines[:3]):  # Max 3 lines for 32px height
        display.text(0, y_positions[i], 1, line)
    
    display.display()


def test_pattern(display):
    """Display a test pattern"""
    display.clear()
    
    # Text
    display.text(0, 0, 1, "LCD4Linux Ready!")
    display.text(0, 10, 1, "ESP32-C3 OLED")
    
    # Line
    display.line(0, 22, 127, 22)
    
    # Box
    display.rect(0, 24, 40, 7, 0)
    
    display.display()


def main():
    parser = argparse.ArgumentParser(description='LCD4Linux to ESP32-C3 Bridge')
    parser.add_argument('--port', default='/dev/ttyACM0', help='Serial port (default: /dev/ttyACM0)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--mode', choices=['system', 'custom', 'test'], default='system',
                        help='Display mode (default: system)')
    parser.add_argument('--interval', type=float, default=2.5, help='Update interval in seconds (default: 2.5)')
    parser.add_argument('--once', action='store_true', help='Run once and exit')
    
    args = parser.parse_args()
    
    # Check if port exists
    if not os.path.exists(args.port):
        print(f"Error: Serial port {args.port} not found")
        print("Available ports:")
        import glob
        ports = glob.glob('/dev/ttyACM*') + glob.glob('/dev/ttyUSB*')
        for port in ports:
            print(f"  {port}")
        sys.exit(1)
    
    try:
        # Connect to ESP32
        print(f"Connecting to {args.port}...")
        display = ESP32Display(args.port, args.baud)
        print("Connected!")
        
        # Get display info
        info = display.get_info()
        print(f"Display info: {info}")
        
        if args.mode == 'test':
            print("Running test pattern...")
            test_pattern(display)
            if not args.once:
                print("Test pattern displayed. Press Ctrl+C to exit.")
                while True:
                    time.sleep(1)
        
        elif args.mode == 'system':
            print("Displaying system info... Press Ctrl+C to exit.")
            while True:
                draw_system_info(display)
                if args.once:
                    break
                time.sleep(args.interval)
        
        elif args.mode == 'custom':
            print("Custom mode: Reading lines from stdin...")
            print("Enter 3 lines of text (empty line to update):")
            lines = []
            while True:
                try:
                    line = input()
                    if line == '':
                        if lines:
                            draw_custom_text(display, lines)
                            lines = []
                    else:
                        lines.append(line)
                        if len(lines) >= 3:
                            draw_custom_text(display, lines)
                            lines = []
                except EOFError:
                    break
        
        display.close()
        print("\nDisconnected.")
    
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nExiting...")
        display.close()
        sys.exit(0)
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
