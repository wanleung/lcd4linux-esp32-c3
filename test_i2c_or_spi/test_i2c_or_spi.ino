/*
 * OLED Display Protocol Test - I2C or SPI?
 * This sketch tests if your OLED is I2C or SPI
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// I2C Configuration
#define I2C_SDA 8
#define I2C_SCL 9

// Common ESP32-C3 SPI pins
#define SPI_MOSI 6   // Data
#define SPI_CLK  7   // Clock
#define SPI_DC   4   // Data/Command
#define SPI_CS   5   // Chip Select
#define SPI_RST  3   // Reset (or use -1 if not connected)

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== OLED Protocol Detection Test ===");
  Serial.println("Testing if your display is I2C or SPI...\n");
  
  // Test 1: Try I2C first
  Serial.println("TEST 1: I2C Protocol");
  Serial.println("-------------------");
  if (testI2C()) {
    Serial.println("\n✓ SUCCESS! Your display is I2C");
    Serial.println("\nUse these settings:");
    Serial.print("  SDA -> GPIO"); Serial.println(I2C_SDA);
    Serial.print("  SCL -> GPIO"); Serial.println(I2C_SCL);
    Serial.println("  VCC -> 3.3V");
    Serial.println("  GND -> GND");
    Serial.println("\nYou can now use lcd4linux_esp32.ino");
    while(1) {
      delay(1000);
      Serial.println("✓ I2C Display Working!");
    }
  }
  
  Serial.println("✗ I2C test failed\n");
  delay(1000);
  
  // Test 2: Try SPI
  Serial.println("TEST 2: SPI Protocol");
  Serial.println("-------------------");
  if (testSPI()) {
    Serial.println("\n✓ SUCCESS! Your display is SPI");
    Serial.println("\nYour display uses SPI protocol.");
    Serial.println("The current project supports I2C only.");
    Serial.println("\nTo use SPI display:");
    Serial.println("1. You need different wiring (5-7 wires)");
    Serial.println("2. The code needs modification for SPI");
    Serial.println("\nSPI pins detected:");
    Serial.print("  MOSI -> GPIO"); Serial.println(SPI_MOSI);
    Serial.print("  CLK  -> GPIO"); Serial.println(SPI_CLK);
    Serial.print("  DC   -> GPIO"); Serial.println(SPI_DC);
    Serial.print("  CS   -> GPIO"); Serial.println(SPI_CS);
    Serial.print("  RST  -> GPIO"); Serial.println(SPI_RST);
    while(1) {
      delay(1000);
      Serial.println("✓ SPI Display Working!");
    }
  }
  
  Serial.println("✗ SPI test failed\n");
  
  // Both failed
  Serial.println("\n=== BOTH TESTS FAILED ===");
  Serial.println("\nPossible issues:");
  Serial.println("1. Display not powered (check VCC/GND)");
  Serial.println("2. Wrong pin connections");
  Serial.println("3. Display may be damaged");
  Serial.println("4. Not a SSD1306 display");
  Serial.println("\nCheck your display label for:");
  Serial.println("  - Model number (should be SSD1306)");
  Serial.println("  - Pin labels (SDA/SCL = I2C, MOSI/SCK/DC/CS = SPI)");
  Serial.println("  - Voltage (should be 3.3V compatible)");
}

bool testI2C() {
  Serial.print("Initializing I2C on SDA=GPIO"); Serial.print(I2C_SDA);
  Serial.print(", SCL=GPIO"); Serial.print(I2C_SCL); Serial.println("...");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);
  
  // Scan for devices
  Serial.println("Scanning I2C bus...");
  byte deviceCount = 0;
  byte foundAddress = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("  Found device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      deviceCount++;
      foundAddress = address;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("  No I2C devices found");
    return false;
  }
  
  // Try to initialize display with found address
  Serial.print("Attempting to initialize display at 0x");
  if (foundAddress < 16) Serial.print("0");
  Serial.print(foundAddress, HEX); Serial.println("...");
  
  Adafruit_SSD1306 display_i2c(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  // Try 0x3C first
  if (display_i2c.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("  Display initialized at 0x3C!");
    display_i2c.clearDisplay();
    display_i2c.setTextSize(1);
    display_i2c.setTextColor(SSD1306_WHITE);
    display_i2c.setCursor(0, 0);
    display_i2c.println("I2C Display");
    display_i2c.println("Working!");
    display_i2c.println("Address: 0x3C");
    display_i2c.display();
    return true;
  }
  
  // Try 0x3D
  if (display_i2c.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println("  Display initialized at 0x3D!");
    display_i2c.clearDisplay();
    display_i2c.setTextSize(1);
    display_i2c.setTextColor(SSD1306_WHITE);
    display_i2c.setCursor(0, 0);
    display_i2c.println("I2C Display");
    display_i2c.println("Working!");
    display_i2c.println("Address: 0x3D");
    display_i2c.display();
    Serial.println("  NOTE: Update config.h to use 0x3D");
    return true;
  }
  
  Serial.println("  Display init failed (device found but not SSD1306?)");
  return false;
}

bool testSPI() {
  Serial.print("Initializing SPI on MOSI=GPIO"); Serial.print(SPI_MOSI);
  Serial.print(", CLK=GPIO"); Serial.print(SPI_CLK);
  Serial.print(", DC=GPIO"); Serial.print(SPI_DC);
  Serial.print(", CS=GPIO"); Serial.print(SPI_CS);
  Serial.print(", RST=GPIO"); Serial.print(SPI_RST); Serial.println("...");
  
  // Create SPI display object
  Adafruit_SSD1306 display_spi(SCREEN_WIDTH, SCREEN_HEIGHT,
    SPI_MOSI, SPI_CLK, SPI_DC, SPI_RST, SPI_CS);
  
  Serial.println("Attempting to initialize SPI display...");
  
  if (display_spi.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("  Display initialized!");
    display_spi.clearDisplay();
    display_spi.setTextSize(1);
    display_spi.setTextColor(SSD1306_WHITE);
    display_spi.setCursor(0, 0);
    display_spi.println("SPI Display");
    display_spi.println("Working!");
    display_spi.display();
    return true;
  }
  
  Serial.println("  SPI init failed");
  return false;
}

void loop() {
  // Nothing here
}
