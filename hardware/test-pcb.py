#!/usr/bin/env python3
"""
PCB Testing and Verification Script
Tests ESP32-C3 OLED board functionality
"""

import serial
import time
import sys

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def print_test(name, result, details=""):
    symbol = f"{Colors.GREEN}✓{Colors.END}" if result else f"{Colors.RED}✗{Colors.END}"
    print(f"{symbol} {name}")
    if details:
        print(f"  → {details}")

def test_pcb(port='/dev/ttyACM0', baudrate=115200):
    """Run complete PCB verification tests"""
    
    print(f"\n{Colors.BLUE}{'='*50}{Colors.END}")
    print(f"{Colors.BLUE}ESP32-C3 OLED PCB Verification{Colors.END}")
    print(f"{Colors.BLUE}{'='*50}{Colors.END}\n")
    
    print(f"Testing port: {port} @ {baudrate} baud\n")
    
    tests_passed = 0
    tests_total = 7
    
    try:
        # Test 1: Serial port connection
        print("Test 1: Opening serial port...")
        try:
            ser = serial.Serial(port, baudrate, timeout=2)
            time.sleep(2)  # Wait for ESP32 to reset
            print_test("Serial port opened", True, f"{port} at {baudrate} baud")
            tests_passed += 1
        except Exception as e:
            print_test("Serial port opened", False, str(e))
            return False
        
        # Clear any startup messages
        while ser.in_waiting:
            ser.readline()
        
        # Test 2: PING command
        print("\nTest 2: Communication test (PING)...")
        ser.write(b"PING\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "PONG" in response:
                print_test("PING response", True, response)
                tests_passed += 1
            else:
                print_test("PING response", False, f"Expected PONG, got: {response}")
        else:
            print_test("PING response", False, "No response")
        
        # Test 3: INFO command
        print("\nTest 3: Display info (INFO)...")
        ser.write(b"INFO\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "OK:INFO" in response:
                print_test("Display info", True, response)
                tests_passed += 1
            else:
                print_test("Display info", False, f"Unexpected response: {response}")
        else:
            print_test("Display info", False, "No response")
        
        # Test 4: CLEAR command
        print("\nTest 4: Clear display (CLEAR)...")
        ser.write(b"CLEAR\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "OK" in response:
                print_test("Clear display", True, response)
                tests_passed += 1
            else:
                print_test("Clear display", False, f"Unexpected response: {response}")
        else:
            print_test("Clear display", False, "No response")
        
        # Test 5: TEXT command
        print("\nTest 5: Text display (TEXT)...")
        ser.write(b"TEXT:0:0:1:PCB Test\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "OK" in response:
                print_test("Text display", True, "Text command accepted")
                tests_passed += 1
            else:
                print_test("Text display", False, f"Unexpected response: {response}")
        else:
            print_test("Text display", False, "No response")
        
        # Test 6: LINE command
        print("\nTest 6: Line drawing (LINE)...")
        ser.write(b"LINE:0:20:127:20\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "OK" in response:
                print_test("Line drawing", True, "Line command accepted")
                tests_passed += 1
            else:
                print_test("Line drawing", False, f"Unexpected response: {response}")
        else:
            print_test("Line drawing", False, "No response")
        
        # Test 7: DISPLAY update
        print("\nTest 7: Display update (DISPLAY)...")
        ser.write(b"DISPLAY\n")
        ser.flush()
        time.sleep(0.1)
        
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore').strip()
            if "OK" in response:
                print_test("Display update", True, "Display refreshed successfully")
                tests_passed += 1
            else:
                print_test("Display update", False, f"Unexpected response: {response}")
        else:
            print_test("Display update", False, "No response")
        
        # Close serial port
        ser.close()
        
        # Summary
        print(f"\n{Colors.BLUE}{'='*50}{Colors.END}")
        print(f"\n{Colors.BLUE}Test Summary:{Colors.END}")
        print(f"  Passed: {tests_passed}/{tests_total}")
        print(f"  Failed: {tests_total - tests_passed}/{tests_total}")
        
        if tests_passed == tests_total:
            print(f"\n{Colors.GREEN}{'='*50}")
            print(f"✓ ALL TESTS PASSED - PCB is working correctly!")
            print(f"{'='*50}{Colors.END}\n")
            return True
        else:
            print(f"\n{Colors.YELLOW}{'='*50}")
            print(f"⚠ Some tests failed - check connections")
            print(f"{'='*50}{Colors.END}\n")
            return False
        
    except serial.SerialException as e:
        print_test("Serial communication", False, str(e))
        print(f"\n{Colors.RED}Serial port error. Check:{Colors.END}")
        print(f"  - ESP32-C3 is connected")
        print(f"  - Correct port ({port})")
        print(f"  - Arduino sketch is uploaded")
        print(f"  - User has permission (add to dialout group)")
        return False
    
    except KeyboardInterrupt:
        print(f"\n\n{Colors.YELLOW}Test interrupted by user{Colors.END}")
        return False
    
    except Exception as e:
        print_test("PCB test", False, str(e))
        import traceback
        traceback.print_exc()
        return False


def display_pattern():
    """Display a test pattern on OLED"""
    port = '/dev/ttyACM0'
    
    print(f"\n{Colors.BLUE}Displaying test pattern...{Colors.END}\n")
    
    try:
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)
        
        # Clear
        ser.write(b"CLEAR\n")
        ser.readline()
        
        # Title
        ser.write(b"TEXT:0:0:1:PCB Test OK!\n")
        ser.readline()
        
        # Info
        ser.write(b"TEXT:0:10:1:ESP32-C3 OLED\n")
        ser.readline()
        
        # Line
        ser.write(b"LINE:0:22:127:22\n")
        ser.readline()
        
        # Box
        ser.write(b"RECT:0:24:40:7:0\n")
        ser.readline()
        
        # Update
        ser.write(b"DISPLAY\n")
        response = ser.readline().decode().strip()
        
        if "OK" in response:
            print(f"{Colors.GREEN}✓ Test pattern displayed{Colors.END}")
        
        ser.close()
        
    except Exception as e:
        print(f"{Colors.RED}✗ Error: {e}{Colors.END}")


if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='ESP32-C3 OLED PCB Testing')
    parser.add_argument('--port', default='/dev/ttyACM0', help='Serial port')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate')
    parser.add_argument('--pattern', action='store_true', help='Display test pattern')
    
    args = parser.parse_args()
    
    if args.pattern:
        display_pattern()
    else:
        success = test_pcb(args.port, args.baud)
        sys.exit(0 if success else 1)
