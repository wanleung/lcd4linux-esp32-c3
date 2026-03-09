/*
 * Configuration file for LCD4Linux ESP32-C3 Driver
 * 
 * Modify these settings according to your hardware setup
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== DISPLAY SETTINGS =====
// OLED display dimensions (128x32)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// OLED I2C address (usually 0x3C for 128x32)
#define SCREEN_ADDRESS 0x3C

// OLED reset pin (-1 if sharing Arduino reset pin)
#define OLED_RESET -1


// ===== ESP32-C3 I2C PIN CONFIGURATION =====
// Default I2C pins for ESP32-C3
// You can change these to any available GPIO pins

// SDA (Data line) - Default GPIO8
#define I2C_SDA 8

// SCL (Clock line) - Default GPIO9  
#define I2C_SCL 9

/*
 * Alternative ESP32-C3 GPIO pins you can use:
 * GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7
 * GPIO8, GPIO9, GPIO10, GPIO18, GPIO19, GPIO20, GPIO21
 * 
 * Note: Avoid GPIO18/19 if using USB as they're USB D-/D+
 */


// ===== SERIAL COMMUNICATION SETTINGS =====
// Baud rate for USB Serial communication with lcd4linux daemon
#define BAUD_RATE 115200

/*
 * On ESP32-C3, USB CDC (USB Serial) is available on GPIO18/GPIO19
 * This is automatically handled by the Serial object in Arduino
 * No need to specify USB pins manually
 */


// ===== I2C SETTINGS =====
// I2C clock frequency (Hz)
// Standard: 100000 (100kHz)
// Fast: 400000 (400kHz)
// Default is 100kHz, you can increase for faster updates
#define I2C_FREQUENCY 400000


// ===== ADVANCED SETTINGS =====
// Enable debug output (set to 1 for debugging)
#define DEBUG_MODE 0

// Display update timeout (ms)
#define DISPLAY_TIMEOUT 5000

// Command buffer size
#define CMD_BUFFER_SIZE 256

#endif // CONFIG_H
