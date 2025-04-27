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
  delay(300);          // Give time for HM-10 to reply
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
  sendATCommand("AT+BAUD0");  // Set baud rate to 9600
  sendATCommand("AT+ROLE0");  // Set to slave role
}


#define FLUID_PARTICLES 30
// Structures
struct Vector2D {
  float x;
  float y;
};

struct Particle {
  Vector2D position;
  Vector2D velocity;
};
Particle particles[FLUID_PARTICLES];
Vector2D acceleration = {0, 0};

// Adjusted constants for smoother motion
const float GRAVITY = 0.3f;    //0.03f
const float DAMPING = 0.60f;   //   0.98f
const float MAX_VELOCITY = 0.7f;  //0.3f
const float MIN_MOVEMENT = 0.001f;  //0.001f


void drawParticles() {
  matrix.clear();
  
  // Create occupancy grid
  bool occupied[matrix.width()][matrix.height()] = {{false}};
  
  // Get and smooth gravity direction
  static Vector2D lastGravityDir = {0, 1};
  Vector2D currentGravityDir;
  
  
  currentGravityDir = acceleration;
  
  
  const float GRAVITY_SMOOTHING = 0.95f;
  lastGravityDir.x = lastGravityDir.x * GRAVITY_SMOOTHING + currentGravityDir.x * (1 - GRAVITY_SMOOTHING);
  lastGravityDir.y = lastGravityDir.y * GRAVITY_SMOOTHING + currentGravityDir.y * (1 - GRAVITY_SMOOTHING);
  
  // Normalize gravity vector
  float gravMagnitude = sqrt(lastGravityDir.x * lastGravityDir.x + lastGravityDir.y * lastGravityDir.y);
  Vector2D gravityDir = {0, 1}; // Default down direction
  
  if (gravMagnitude > 0.1f) {
      gravityDir.x = lastGravityDir.x / gravMagnitude;
      gravityDir.y = lastGravityDir.y / gravMagnitude;
  }

  // Calculate heights and prepare for drawing
  float heights[FLUID_PARTICLES];
  float minHeight = 1000;
  float maxHeight = -1000;
  
  for (int i = 0; i < FLUID_PARTICLES; i++) {
      heights[i] = -(particles[i].position.x * gravityDir.x + 
                    particles[i].position.y * gravityDir.y);
      minHeight = min(minHeight, heights[i]);
      maxHeight = max(maxHeight, heights[i]);
  }
  
  float heightRange = max(maxHeight - minHeight, 1.0f);

  // Draw particles
  int visibleCount = 0;
  for (int i = 0; i < FLUID_PARTICLES; i++) {
      int x = round(constrain(particles[i].position.x, 0, matrix.width() - 1));
      int y = round(constrain(particles[i].position.y, 0, matrix.height() - 1));
      
      if (!occupied[x][y]) {
          
          }
      } else {
          // Find nearest empty position
          for (int dx = -1; dx <= 1; dx++) {
              for (int dy = -1; dy <= 1; dy++) {
                  if (dx == 0 && dy == 0) continue;
                  
                  int newX = x + dx;
                  int newY = y + dy;
                  
                  if (newX >= 0 && newX < MATRIX_WIDTH && 
                      newY >= 0 && newY < MATRIX_HEIGHT && 
                      !occupied[newX][newY]) {
                      int index = xy(newX, newY);
                      if (index >= 0 && index < NUM_LEDS) {
                          float relativeHeight = (heights[i] - minHeight) / heightRange;
                      uint8_t hue = relativeHeight * 160;  // Map from 0 (red) to 160 (blue)
                      uint8_t sat = 255;  // Full saturation for vibrant colors
                      uint8_t val = 220 + (relativeHeight * 35);  // Slightly brighter at top

                          
                          leds[index] = CHSV(hue, sat, val);
                          occupied[newX][newY] = true;
                          visibleCount++;
                          goto particleDrawn;
                      }
                  }
              }
          }
          particleDrawn: continue;
      }
  }
  
  // Debug output
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000) {
      Serial.printf("Visible particles: %d of %d\n", visibleCount, FLUID_PARTICLES);
      lastDebugTime = millis();
  }
  
  FastLED.show();
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
  }
  else if (cmd.startsWith("FOR:")) {
    int commaIndex = cmd.indexOf(',');
    if (commaIndex > 4) {
      forceX = cmd.substring(4, commaIndex).toInt();
      forceY = cmd.substring(commaIndex + 1).toInt();
      Serial.print("Force set to: ");
      Serial.print(forceX);
      Serial.print(", ");
      Serial.println(forceY);
    }
  } else if (cmd.equalsIgnoreCase("RESET")) {
    initMarbles();
    Serial.println("Marbles reset.");
  }  else {
    btSerial.println("Unknown command.");
  }
}


// Handle incoming Bluetooth commands
void handleBluetooth() {
  if (btSerial.available()) {
    String inputBuffer = btSerial.readStringUntil('\n');
    inputBuffer.trim();  // Removes \r or spaces if any

    if (inputBuffer.length() > 0) {
      Serial.println("Received: " + inputBuffer);
      processCommand(inputBuffer);
    }
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
  btSerial.begin(9600);

  configureBluetooth(); // Configure HM-10 at boot

  matrix.begin();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);

  initMarbles();

  Serial.println("Setup done.");
}

// Main loop
void loop() {
  handleBluetooth();
  // updateDisplay();
  
  unsigned long now = millis();
  if (now - lastUpdate > 30) {
    lastUpdate = now;
    updateMarbles();
    drawMarbles();
  }
}
