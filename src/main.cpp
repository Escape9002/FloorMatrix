#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/TomThumb.h>
#include <SoftwareSerial.h>

#include "state_machine.cpp"

// #define DEBUG 1
#if DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_BEGIN(x) Serial.begin(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#define DEBUG_BEGIN(x)
#endif

#include "BLEDriver.h"
BLEInterface *ble_driver = nullptr;

// Pin definitions
#define MATRIX_PIN 12

// Create objects
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
    "Nano Rocks!"};
int numMessages = 3;

uint16_t colors[] = {
    matrix.Color(255, 0, 0),
    matrix.Color(0, 255, 0),
    matrix.Color(0, 0, 255),
    matrix.Color(255, 255, 0),
    matrix.Color(0, 255, 255)};

int currentMessage = 0;
int16_t scrollX = 0;
uint32_t scrollTimer = 0;
const uint16_t scrollSpeed = 50; // ms per scroll step

struct Pixel
{
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};



// Update the matrix display
void updateDisplay()
{
  uint32_t now = millis();

  if (now - scrollTimer > scrollSpeed && matrix_mode == MESSAGES)
  {
    scrollTimer = now;

    matrix.fillScreen(0);
    int16_t textWidth = 0;
    switch (font_mode)
    {
    case BIG:
      matrix.setCursor(scrollX, 0);                      // Bottom alignment for TomThumb
      textWidth = messages[currentMessage].length() * 6; // TomThumb is ~4px per char
      break;
    case SMALL:
      matrix.setCursor(scrollX, matrix.height() - 1);    // Bottom alignment for TomThumb
      textWidth = messages[currentMessage].length() * 4; // TomThumb is ~4px per char
      break;
    default:
      matrix.setCursor(scrollX, matrix.height() - 1);    // Bottom alignment for TomThumb
      textWidth = messages[currentMessage].length() * 4; // TomThumb is ~4px per char
      break;
    }

    matrix.print(messages[currentMessage]);
    matrix.show();

    scrollX--;

    if (scrollX < -textWidth)
    {
      scrollX = matrix.width();
      currentMessage++;
      if (currentMessage >= numMessages)
      {
        currentMessage = 0;
      }
      matrix.setTextColor(colors[random(0, 5)]);
    }
  }
}

void setup()
{
  DEBUG_BEGIN(115200);
  // while (!Serial)
  // {
  // }

  DEBUG_PRINTLN("Starting setup ...");

  DEBUG_PRINTLN("\tmatrix setup...");
  matrix.begin();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);
  matrix.setBrightness(25);
  matrix.setTextColor(colors[0]);
  matrix.clear();
  matrix.show();

  matrix.print("Booting...");
  matrix.show();

  DEBUG_PRINTLN("\tBLE setup...");
  ble_driver = &getBLEDriverInstance();
  while (!ble_driver->begin())
  {
    DEBUG_PRINTLN("\t\tcouldnt connect to BLE");
    delay(50);
  }

  DEBUG_PRINTLN("Setup done.");

  matrix.print("Done!");
  matrix.show();
  delay(500);
  matrix.clear();
  matrix.show();
}

String msg = "";

void loop()
{
  if (ble_driver->connected())
  {
    if (ble_driver->available())
    {
      msg = msg + ble_driver->get_received();
      DEBUG_PRINTLN(msg);

      if (msg.length() > 0 && msg.endsWith(">"))
      {
        msg = msg.substring(1, msg.length() - 1);
        DEBUG_PRINTLN(msg);
        processCommand(msg);

        msg = "";
      }
    }
  }
  updateDisplay();
}
