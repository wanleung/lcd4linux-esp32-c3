/*
 * LCD4Linux ESP32-C3 Driver
 * 
 * This sketch enables ESP32-C3 to act as a display driver for lcd4linux daemon
 * via USB Serial and Bluetooth BLE communication. Supports I2C OLED 128x32 displays (SSD1306).
 * 
 * Protocol: lcd4linux sends commands via Serial or BLE, ESP32 renders to OLED
 * Baud Rate: 115200 (Serial)
 * BLE: Custom UART service
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"

// BLE Service UUIDs (Nordic UART Service)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buffer for incoming commands (shared by Serial and BLE)
String serialBuffer = "";
String bleBuffer = "";
const int MAX_BUFFER_SIZE = 256;

// BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Forward declarations
void sendResponse(String response, bool viaBLE);
void processCommand(String cmd, bool viaBLE);

// BLE Server callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE:CLIENT_CONNECTED");
      
      // Show BLE status on display
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("BLE Connected!");
      display.display();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE:CLIENT_DISCONNECTED");
      
      // Show disconnection on display
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("BLE Disconnected");
      display.display();
    }
};

// BLE Characteristic callbacks for receiving data
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* data = pCharacteristic->getData();
      size_t len = pCharacteristic->getValue().length();

      if (len > 0) {
        for (size_t i = 0; i < len; i++) {
          char inChar = (char)data[i];
          
          // Command end delimiter
          if (inChar == '\n' || inChar == '\r') {
            if (bleBuffer.length() > 0) {
              processCommand(bleBuffer, true); // true = BLE source
              bleBuffer = "";
            }
          } else {
            bleBuffer += inChar;
            // Prevent buffer overflow
            if (bleBuffer.length() >= MAX_BUFFER_SIZE) {
              sendResponse("ERR:BUFFER_OVERFLOW", true);
              bleBuffer = "";
            }
          }
        }
      }
    }
};

void sendResponse(String response, bool viaBLE) {
  if (viaBLE && deviceConnected) {
    // Send via BLE
    pTxCharacteristic->setValue(response.c_str());
    pTxCharacteristic->notify();
  } else {
    // Send via Serial
    Serial.println(response);
  }
}

void setup() {
  // Initialize Serial communication for lcd4linux
  Serial.begin(BAUD_RATE);
  // Don't wait for Serial on ESP32-C3 USB - it might not connect immediately
  delay(1000); // Just a short delay for Serial to stabilize
  
  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERR:DISPLAY_INIT_FAILED");
    // Don't halt, continue anyway and report error
  } else {
    Serial.println("OK:DISPLAY_INITIALIZED");
  }
  
  // Clear the display buffer and show ready message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LCD4Linux Ready");
  display.println("ESP32-C3");
  display.println("Serial + BLE");
  display.display();
  
  // Send ready signal to lcd4linux
  Serial.println("OK:READY");
  serialBuffer.reserve(MAX_BUFFER_SIZE);
  bleBuffer.reserve(MAX_BUFFER_SIZE);
  
  // Initialize BLE
  initBLE();
  Serial.println("BLE:INITIALIZED");
}

void initBLE() {
  // Create BLE Device with custom name
  BLEDevice::init(BLE_DEVICE_NAME);
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic for TX (ESP32 -> Client)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Create BLE Characteristic for RX (Client -> ESP32)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void loop() {
  // Check for incoming serial data
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    
    // Command end delimiter
    if (inChar == '\n' || inChar == '\r') {
      if (serialBuffer.length() > 0) {
        processCommand(serialBuffer, false); // false = Serial source
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
  
  // Handle BLE connection state changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack time to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("BLE:START_ADVERTISING");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void processCommand(String cmd, bool viaBLE) {
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
    sendResponse("OK:CLEAR", viaBLE);
    
  } else if (command == "TEXT") {
    // Format: TEXT:x:y:size:text
    handleTextCommand(params, viaBLE);
    
  } else if (command == "PIXEL") {
    // Format: PIXEL:x:y:state
    handlePixelCommand(params, viaBLE);
    
  } else if (command == "LINE") {
    // Format: LINE:x0:y0:x1:y1
    handleLineCommand(params, viaBLE);
    
  } else if (command == "RECT") {
    // Format: RECT:x:y:w:h:fill
    handleRectCommand(params, viaBLE);
    
  } else if (command == "CIRCLE") {
    // Format: CIRCLE:x:y:r:fill
    handleCircleCommand(params, viaBLE);
    
  } else if (command == "DISPLAY") {
    // Update display with buffer content
    display.display();
    sendResponse("OK:DISPLAY", viaBLE);
    
  } else if (command == "PING") {
    // Keep-alive
    sendResponse("OK:PONG", viaBLE);
    
  } else if (command == "INFO") {
    // Send display info
    String info = "OK:INFO:" + String(SCREEN_WIDTH) + "x" + String(SCREEN_HEIGHT) + ":I2C:" + String(I2C_SDA) + "," + String(I2C_SCL);
    if (viaBLE) {
      info += ":BLE";
    }
    sendResponse(info, viaBLE);
    
  } else if (command == "TEST") {
    // Show test pattern
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("TEST PATTERN");
    display.setCursor(0, 10);
    display.print("Addr: 0x");
    display.println(SCREEN_ADDRESS, HEX);
    display.setCursor(0, 20);
    display.print("I2C: ");
    display.print(I2C_SDA);
    display.print(",");
    display.print(I2C_SCL);
    display.display();
    sendResponse("OK:TEST", viaBLE);
    
  } else {
    sendResponse("ERR:UNKNOWN_COMMAND", viaBLE);
  }
}

void handleTextCommand(String params, bool viaBLE) {
  // Parse: x:y:size:text
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    sendResponse("ERR:TEXT_INVALID_PARAMS", viaBLE);
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int size = params.substring(idx2 + 1, idx3).toInt();
  String text = params.substring(idx3 + 1);
  
  display.setTextSize(size > 0 ? size : 1);
  display.setCursor(x, y);
  display.print(text);
  
  sendResponse("OK:TEXT", viaBLE);
}

void handlePixelCommand(String params, bool viaBLE) {
  // Parse: x:y:state
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  
  if (idx1 < 0 || idx2 < 0) {
    sendResponse("ERR:PIXEL_INVALID_PARAMS", viaBLE);
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int state = params.substring(idx2 + 1).toInt();
  
  display.drawPixel(x, y, state ? SSD1306_WHITE : SSD1306_BLACK);
  sendResponse("OK:PIXEL", viaBLE);
}

void handleLineCommand(String params, bool viaBLE) {
  // Parse: x0:y0:x1:y1
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    sendResponse("ERR:LINE_INVALID_PARAMS", viaBLE);
    return;
  }
  
  int x0 = params.substring(0, idx1).toInt();
  int y0 = params.substring(idx1 + 1, idx2).toInt();
  int x1 = params.substring(idx2 + 1, idx3).toInt();
  int y1 = params.substring(idx3 + 1).toInt();
  
  display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  sendResponse("OK:LINE", viaBLE);
}

void handleRectCommand(String params, bool viaBLE) {
  // Parse: x:y:w:h:fill[:color] - color is optional (0=black, 1=white, default=white)
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  int idx4 = params.indexOf(':', idx3 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0 || idx4 < 0) {
    sendResponse("ERR:RECT_INVALID_PARAMS", viaBLE);
    return;
  }
  
  int x = params.substring(0, idx1).toInt();
  int y = params.substring(idx1 + 1, idx2).toInt();
  int w = params.substring(idx2 + 1, idx3).toInt();
  int h = params.substring(idx3 + 1, idx4).toInt();
  
  // Get fill parameter
  String remaining = params.substring(idx4 + 1);
  int idx5 = remaining.indexOf(':');
  int fill = (idx5 > 0) ? remaining.substring(0, idx5).toInt() : remaining.toInt();
  
  // Get optional color parameter (default to white for backwards compatibility)
  int color = (idx5 > 0) ? remaining.substring(idx5 + 1).toInt() : 1;
  uint16_t drawColor = (color == 0) ? SSD1306_BLACK : SSD1306_WHITE;
  
  if (fill) {
    display.fillRect(x, y, w, h, drawColor);
  } else {
    display.drawRect(x, y, w, h, drawColor);
  }
  
  sendResponse("OK:RECT", viaBLE);
}

void handleCircleCommand(String params, bool viaBLE) {
  // Parse: x:y:r:fill
  int idx1 = params.indexOf(':');
  int idx2 = params.indexOf(':', idx1 + 1);
  int idx3 = params.indexOf(':', idx2 + 1);
  
  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    sendResponse("ERR:CIRCLE_INVALID_PARAMS", viaBLE);
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
  
  sendResponse("OK:CIRCLE", viaBLE);
}
