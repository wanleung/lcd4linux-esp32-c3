/*
 * LCD4Linux ESP32-C3 Driver
 * 
 * This sketch enables ESP32-C3 to act as a display driver for lcd4linux daemon
 * via USB Serial communication. Supports I2C OLED 128x32 displays (SSD1306).
 * 
 * Protocol: lcd4linux sends commands via Serial, ESP32 renders to OLED
 * Baud Rate: 115200
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buffer for incoming serial commands
String serialBuffer = "";
const int MAX_BUFFER_SIZE = 256;

void setup() {
  // Initialize Serial communication for lcd4linux
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }
  
  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERR:DISPLAY_INIT_FAILED");
    while(1); // Don't proceed, loop forever
  }
  
  // Clear the display buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LCD4Linux Ready");
  display.println("ESP32-C3");
  display.display();
  
  // Send ready signal to lcd4linux
  Serial.println("OK:READY");
  serialBuffer.reserve(MAX_BUFFER_SIZE);
}

void loop() {
  // Check for incoming serial data
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    
    // Command end delimiter
    if (inChar == '\n' || inChar == '\r') {
      if (serialBuffer.length() > 0) {
        processCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += inChar;
      // Prevent buffer overflow
      if (serialBuffer.length() >= MAX_BUFFER_SIZE) {
        Serial.println("ERR:BUFFER_OVERFLOW");
        serialBuffer = "";
      }
    }
  }
}

void processCommand(String cmd) {
  cmd.trim();
  
  if (cmd.length() == 0) {
    return;
  }
  
  // Parse command format: CMD:param1:param2:...
  int firstColon = cmd.indexOf(':');
  String command = (firstColon > 0) ? cmd.substring(0, firstColon) : cmd;
  String params = (firstColon > 0) ? cmd.substring(firstColon + 1) : "";
  
  // Command processing
  if (command == "CLEAR") {
    // Clear display
    display.clearDisplay();
    display.display();
    Serial.println("OK:CLEAR");
    
  } else if (command == "TEXT") {
    // Format: TEXT:x:y:size:text
    handleTextCommand(params);
    
  } else if (command == "PIXEL") {
    // Format: PIXEL:x:y:state
    handlePixelCommand(params);
    
  } else if (command == "LINE") {
    // Format: LINE:x0:y0:x1:y1
    handleLineCommand(params);
    
  } else if (command == "RECT") {
    // Format: RECT:x:y:w:h:fill
    handleRectCommand(params);
    
  } else if (command == "CIRCLE") {
    // Format: CIRCLE:x:y:r:fill
    handleCircleCommand(params);
    
  } else if (command == "DISPLAY") {
    // Update display with buffer content
    display.display();
    Serial.println("OK:DISPLAY");
    
  } else if (command == "PING") {
    // Keep-alive
    Serial.println("OK:PONG");
    
  } else if (command == "INFO") {
    // Send display info
    Serial.print("OK:INFO:");
    Serial.print(SCREEN_WIDTH);
    Serial.print("x");
    Serial.print(SCREEN_HEIGHT);
    Serial.print(":I2C:");
    Serial.print(I2C_SDA);
    Serial.print(",");
    Serial.println(I2C_SCL);
    
  } else {
    Serial.println("ERR:UNKNOWN_COMMAND");
  }
}

void handleTextCommand(String params) {
  // Parse: x:y:size:text
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    Serial.println("ERR:TEXT_INVALID_PARAMS");
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int size = params.substring(idx2 + 1, idx3).toInt();
  String text = params.substring(idx3 + 1);
  
  display.setTextSize(size > 0 ? size : 1);
  display.setCursor(x, y);
  display.print(text);
  
  Serial.println("OK:TEXT");
}

void handlePixelCommand(String params) {
  // Parse: x:y:state
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  
  if (idx1 < 0 || idx2 < 0) {
    Serial.println("ERR:PIXEL_INVALID_PARAMS");
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int state = params.substring(idx2 + 1).toInt();
  
  display.drawPixel(x, y, state ? SSD1306_WHITE : SSD1306_BLACK);
  Serial.println("OK:PIXEL");
}

void handleLineCommand(String params) {
  // Parse: x0:y0:x1:y1
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    Serial.println("ERR:LINE_INVALID_PARAMS");
    return;
  }
  
  int x0 = params.substring(0, idx1).toInt();
  int y0 = params.substring(idx1 + 1, idx2).toInt();
  int x1 = params.substring(idx2 + 1, idx3).toInt();
  int y1 = params.substring(idx3 + 1).toInt();
  
  display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  Serial.println("OK:LINE");
}

void handleRectCommand(String params) {
  // Parse: x:y:w:h:fill
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  int idx4 = params.indexOf(':', idx3 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0 || idx4 < 0) {
    Serial.println("ERR:RECT_INVALID_PARAMS");
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int w = params.substring(idx2 + 1, idx3).toInt();
  int h = params.substring(idx3 + 1, idx4).toInt();
  int fill = params.substring(idx4 + 1).toInt();
  
  if (fill) {
    display.fillRect(x, y, w, h, SSD1306_WHITE);
  } else {
    display.drawRect(x, y, w, h, SSD1306_WHITE);
  }
  
  Serial.println("OK:RECT");
}

void handleCircleCommand(String params) {
  // Parse: x:y:r:fill
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    Serial.println("ERR:CIRCLE_INVALID_PARAMS");
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int r = params.substring(idx2 + 1, idx3).toInt();
  int fill = params.substring(idx3 + 1).toInt();
  
  if (fill) {
    display.fillCircle(x, y, r, SSD1306_WHITE);
  } else {
    display.drawCircle(x, y, r, SSD1306_WHITE);
  }
  
  Serial.println("OK:CIRCLE");
}
