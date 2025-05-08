#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/TomThumb.h>
#include <SoftwareSerial.h>

// Pin definitions
#define MATRIX_PIN 12
#define BT_RX 7
#define BT_TX 8

// Create objects
SoftwareSerial btSerial(BT_RX, BT_TX); // SoftwareSerial for Bluetooth
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  32, 8, MATRIX_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

// Message list
#define MAX_MESSAGES 10
String messages[MAX_MESSAGES] = {
  "Hello World!",
  "NeoMatrix!",
  "Nano Rocks!"
};
int numMessages = 3;

uint16_t colors[] = {
  matrix.Color(255, 0, 0),
  matrix.Color(0, 255, 0),
  matrix.Color(0, 0, 255),
  matrix.Color(255, 255, 0),
  matrix.Color(0, 255, 255)
};

int currentMessage = 0;
int16_t scrollX = 0;
uint32_t scrollTimer = 0;
const uint16_t scrollSpeed = 50; // ms per scroll step
void sendATCommand(const char* cmd) {
    btSerial.print(cmd); // No println! Only raw command
    delay(100);          // Give time for HM-10 to reply
    while (btSerial.available()) {
      String response = btSerial.readStringUntil('\n');
      Serial.println("HM-10 Response: " + response);
    }
  }

  void configureBluetooth() {
    Serial.println("Configuring Bluetooth...");
    delay(1000); // Allow HM-10 boot time
  
    sendATCommand("AT");        // Test communication
    sendATCommand("AT+RESET");  // Soft reset
    sendATCommand("AT+NAMENeoMatrix"); // Set device name
    sendATCommand("AT+BAUD1");  // Set baud rate to 9600
    sendATCommand("AT+ROLE0");  // Set to slave role
  }
  
  
  
  // Process Bluetooth command
  void processCommand(String cmd) {
    Serial.println("Received command: " + cmd);
    cmd.trim();

  
    if (cmd.startsWith("ADD:")) {
      if (numMessages < MAX_MESSAGES) {
        messages[numMessages] = cmd.substring(4);
        numMessages++;
        btSerial.println("Added!");
      } else {
        btSerial.println("Message list full!");
      }
    } else if (cmd.startsWith("DEL:")) {
      int idx = cmd.substring(4).toInt();
      if (idx >= 0 && idx < numMessages) {
        for (int i = idx; i < numMessages - 1; i++) {
          messages[i] = messages[i + 1];
        }
        numMessages--;
        btSerial.println("Deleted!");
      } else {
        btSerial.println("Invalid index!");
      }
    } else if (cmd.equalsIgnoreCase("LIST")) {
      btSerial.println("Messages:");
      for (int i = 0; i < numMessages; i++) {
        btSerial.print(i);
        btSerial.print(": ");
        btSerial.println(messages[i]);
      }
    } else {
      btSerial.println("Unknown command.");
    }
  }
  
  String inputBuffer = "";
  unsigned long lastCharTime = 0;
  const unsigned long charTimeout = 5; // ms
  
void handleBluetooth() {
while (btSerial.available()) {
    char c = btSerial.read();
    inputBuffer += c;
    lastCharTime = millis();
}

// If data hasn't arrived for a while, assume end of message
if (inputBuffer.length() > 0 && (millis() - lastCharTime > charTimeout)) {
    inputBuffer.trim();  // Remove whitespace
    Serial.println("Received: " + inputBuffer);
    processCommand(inputBuffer);
    inputBuffer = ""; // Reset for next command
}
}
  
  
  

// Update the matrix display
void updateDisplay() {
  uint32_t now = millis();

  if (now - scrollTimer > scrollSpeed) {
    scrollTimer = now;

    matrix.fillScreen(0);
    matrix.setCursor(scrollX, matrix.height() - 1); // Bottom alignment for TomThumb
    matrix.print(messages[currentMessage]);
    matrix.show();

    scrollX--;

    int16_t textWidth = messages[currentMessage].length() * 4; // TomThumb is ~4px per char
    if (scrollX < -textWidth) {
      scrollX = matrix.width();
      currentMessage++;
      if (currentMessage >= numMessages) {
        currentMessage = 0;
      }
      matrix.setTextColor(colors[random(0, 5)]);
    }
  }
}



// Setup
void setup() {
    Serial.begin(9600);
    btSerial.begin(19200);
  
    configureBluetooth(); // Configure HM-10 at boot
  
    matrix.begin();
    matrix.setFont(&TomThumb);
    matrix.setTextWrap(false);
    matrix.setBrightness(40);
    matrix.setTextColor(colors[0]);
  
    Serial.println("Setup done.");
  }
  
  // Main loop
  void loop() {
    handleBluetooth();
    updateDisplay();
  }
  