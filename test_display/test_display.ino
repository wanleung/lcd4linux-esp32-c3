/*
 * Quick OLED Display Test for ESP32-C3
 * Upload this to verify your display is working
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// Try both common I2C addresses
#define SCREEN_ADDRESS_1 0x3C
#define SCREEN_ADDRESS_2 0x3D

// Your I2C pins from config.h
#define I2C_SDA 3
#define I2C_SCL 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32-C3 OLED Test ===");
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.print("I2C initialized - SDA: GPIO");
  Serial.print(I2C_SDA);
  Serial.print(", SCL: GPIO");
  Serial.println(I2C_SCL);
  
  // Scan for I2C devices
  Serial.println("\nScanning I2C bus...");
  byte deviceCount = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("ERROR: No I2C devices found!");
    Serial.println("\nCheck your wiring:");
    Serial.println("  OLED VCC -> 3.3V");
    Serial.println("  OLED GND -> GND");
    Serial.print("  OLED SDA -> GPIO");
    Serial.println(I2C_SDA);
    Serial.print("  OLED SCL -> GPIO");
    Serial.println(I2C_SCL);
    while(1) delay(1000);
  }
  
  Serial.println("\nTrying to initialize display at 0x3C...");
  if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_1)) {
    Serial.println("SUCCESS! Display initialized at 0x3C");
    testDisplay();
  } else {
    Serial.println("Failed at 0x3C, trying 0x3D...");
    if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_2)) {
      Serial.println("SUCCESS! Display initialized at 0x3D");
      Serial.println("Update config.h: #define SCREEN_ADDRESS 0x3D");
      testDisplay();
    } else {
      Serial.println("ERROR: Could not initialize display!");
      Serial.println("Display found on I2C but init failed.");
      Serial.println("Check if display is really SSD1306 128x32");
      while(1) delay(1000);
    }
  }
}

void testDisplay() {
  // Test 1: Clear and text
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Display Works!");
  display.println("ESP32-C3 Test");
  display.display();
  Serial.println("\nDisplay should show: 'Display Works!'");
  
  delay(3000);
  
  // Test 2: Larger text
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("BIG");
  display.println("TEXT");
  display.display();
  Serial.println("Now showing: 'BIG TEXT'");
  
  delay(3000);
  
  // Test 3: Graphics
  display.clearDisplay();
  display.drawRect(0, 0, 127, 31, SSD1306_WHITE);
  display.drawCircle(64, 16, 10, SSD1306_WHITE);
  display.display();
  Serial.println("Now showing: rectangle and circle");
  
  Serial.println("\n=== Test Complete ===");
  Serial.println("If you see text/graphics, display is working!");
  Serial.println("You can now upload lcd4linux_esp32.ino");
}

void loop() {
  // Blink pattern
  delay(1000);
  display.invertDisplay(true);
  delay(500);
  display.invertDisplay(false);
}
