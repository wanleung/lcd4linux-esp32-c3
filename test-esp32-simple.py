#!/usr/bin/env python3
"""
Simple test to debug ESP32 communication
Shows exactly what is sent and received

Copyright (C) 2024-2026 Wan Leung Wong
Licensed under GNU GPL v3 or later
"""

import serial
import time
import sys

if len(sys.argv) < 2:
    print("Usage: python3 test-esp32-simple.py /dev/ttyACM0")
    sys.exit(1)

PORT = sys.argv[1]

print(f"Connecting to {PORT}...")
ser = serial.Serial(PORT, 115200, timeout=2)

print("Waiting for ESP32 to initialize (5 seconds)...")
time.sleep(5)

# Show any startup messages
print("\n=== Startup Messages ===")
while ser.in_waiting:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    if line:
        print(f"  {line}")
print()

# Test PING command
print("=== Testing PING Command ===")
command = "PING\n"
print(f"Sending: {repr(command)}")
ser.write(command.encode('utf-8'))
ser.flush()

time.sleep(0.5)

print("Responses:")
for i in range(3):  # Try reading up to 3 responses
    if ser.in_waiting:
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        if response:
            print(f"  [{i+1}] {response}")
    else:
        time.sleep(0.3)

# Test INFO command
print("\n=== Testing INFO Command ===")
command = "INFO\n"
print(f"Sending: {repr(command)}")
ser.write(command.encode('utf-8'))
ser.flush()

time.sleep(0.5)

print("Responses:")
for i in range(3):
    if ser.in_waiting:
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        if response:
            print(f"  [{i+1}] {response}")
    else:
        time.sleep(0.3)

ser.close()
print("\nTest complete!")
