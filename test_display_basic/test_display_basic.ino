/*
 * LCD4Linux ESP32-C3 - Display Diagnostic Test
 * 
 * This sketch helps you diagnose display issues:
 * 1. Scans I2C bus to find your display address
 * 2. Tests different I2C pin combinations
 * 3. Attempts to initialize the display
 * 
 * Upload this to troubleshoot dark/blank screen issues
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// Test different pin combinations
struct PinConfig {
  int sda;
  int scl;
  const char* name;
};

PinConfig pinConfigs[] = {
  {3, 4, "GPIO3(SDA), GPIO4(SCL)"},
  {8, 9, "GPIO8(SDA), GPIO9(SCL)"},
  {0, 1, "GPIO0(SDA), GPIO1(SCL)"},
  {5, 6, "GPIO5(SDA), GPIO6(SCL)"},
  {21, 20, "GPIO21(SDA), GPIO20(SCL)"}
};

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n=================================");
  Serial.println("LCD4Linux Display Diagnostic Test");
  Serial.println("=================================\n");
  
  // Test each pin configuration
  for (int i = 0; i < sizeof(pinConfigs)/sizeof(pinConfigs[0]); i++) {
    Serial.println("-----------------------------------");
    Serial.print("Testing pins: ");
    Serial.println(pinConfigs[i].name);
    Serial.println("-----------------------------------");
    
    Wire.begin(pinConfigs[i].sda, pinConfigs[i].scl);
    delay(100);
    
    // Scan I2C bus
    Serial.println("Scanning I2C bus...");
    byte deviceCount = 0;
    byte foundAddress = 0;
    
    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      byte error = Wire.endTransmission();
      
      if (error == 0) {
        Serial.print("  ✓ Device found at 0x");
        if (addr < 16) Serial.print("0");
        Serial.println(addr, HEX);
        deviceCount++;
        foundAddress = addr;
      }
    }
    
    if (deviceCount == 0) {
      Serial.println("  ✗ No I2C devices found");
    } else {
      Serial.print("  Found ");
      Serial.print(deviceCount);
      Serial.println(" device(s)");
      
      // Try to initialize display with found address
      if (foundAddress > 0) {
        Serial.print("\nAttempting to initialize display at 0x");
        if (foundAddress < 16) Serial.print("0");
        Serial.print(foundAddress, HEX);
        Serial.println("...");
        
        Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
        
        if (display.begin(SSD1306_SWITCHCAPVCC, foundAddress)) {
          Serial.println("  ✓✓✓ SUCCESS! Display initialized!");
          Serial.println("\nShowing test pattern on display...");
          
          // Show test pattern
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0, 0);
          display.println("Display Works!");
          display.setCursor(0, 10);
          display.print("Addr: 0x");
          if (foundAddress < 16) display.print("0");
          display.println(foundAddress, HEX);
          display.setCursor(0, 20);
          display.print("SDA:");
          display.print(pinConfigs[i].sda);
          display.print(" SCL:");
          display.print(pinConfigs[i].scl);
          display.display();
          
          Serial.println("\n========== SOLUTION ==========");
          Serial.println("Update your config.h with:");
          Serial.print("  #define SCREEN_ADDRESS 0x");
          if (foundAddress < 16) Serial.print("0");
          Serial.println(foundAddress, HEX);
          Serial.print("  #define I2C_SDA ");
          Serial.println(pinConfigs[i].sda);
          Serial.print("  #define I2C_SCL ");
          Serial.println(pinConfigs[i].scl);
          Serial.println("==============================\n");
          
          Serial.println("Test complete! You can now upload lcd4linux_esp32.ino");
          while(1) delay(1000); // Stop here
          
        } else {
          Serial.println("  ✗ Failed to initialize display");
        }
      }
    }
    
    Wire.end();
    delay(500);
    Serial.println();
  }
  
  Serial.println("=================================");
  Serial.println("Test Complete - No working config found");
  Serial.println("=================================");
  Serial.println("\nPossible issues:");
  Serial.println("1. Check wiring:");
  Serial.println("   - OLED VCC → 3.3V");
  Serial.println("   - OLED GND → GND");
  Serial.println("   - OLED SDA → ESP32 GPIO pin");
  Serial.println("   - OLED SCL → ESP32 GPIO pin");
  Serial.println("2. Check display power (LED should be visible)");
  Serial.println("3. Try a different display");
  Serial.println("4. Verify it's a 128x32 SSD1306 display");
}

void loop() {
  // Nothing here
}
