#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/TomThumb.h>
#include <SoftwareSerial.h>

// #include "state_machine.cpp"

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


enum MATRIX_MODES
{
  PIXEL,
  MESSAGES
};

enum FONT_MODES
{
  SMALL,
  BIG
};
FONT_MODES font_mode = SMALL;

MATRIX_MODES matrix_mode = MESSAGES;

// Process Bluetooth command
void processCommand(String cmd)
{
  DEBUG_PRINTLN("Received command: " + cmd);
  cmd.trim();
  String msg = "";

  if (cmd.startsWith("p:"))
  {

    /**
     * Packet format should be in csv format:
     * p:x,y,z,r,g,b
     */

    matrix_mode = PIXEL;

    Pixel p;

    DEBUG_PRINTLN(cmd);

    cmd = cmd.substring(2, cmd.length());
    DEBUG_PRINTLN(cmd);

    p.x = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    p.y = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    p.z = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    p.r = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    p.g = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    p.b = (uint8_t)cmd.substring(0, cmd.indexOf(",")).toInt();
    cmd = cmd.substring(cmd.indexOf(",") + 1);
    DEBUG_PRINTLN(cmd);

    DEBUG_PRINTLN("assambled:");
    DEBUG_PRINT(p.x);
    DEBUG_PRINT(",");
    DEBUG_PRINT(p.y);
    DEBUG_PRINT(",");
    DEBUG_PRINT(p.r);
    DEBUG_PRINT(",");
    DEBUG_PRINT(p.g);
    DEBUG_PRINT(",");
    DEBUG_PRINT(p.b);
    DEBUG_PRINT(",");

    matrix.drawPixel(p.x, p.y, matrix.Color(p.r, p.g, p.b));
    matrix.show();
    msg = "Pixel";
  }
  else if (cmd.startsWith("CLEAR"))
  {
    matrix.clear();
    matrix.show();
  }
  else if (cmd.startsWith("ADD:"))
  {
    if (numMessages < MAX_MESSAGES)
    {
      messages[numMessages] = cmd.substring(4);
      numMessages++;
      msg = "Added!";
    }
    else
    {
      msg = "Message list full!";
    }
    matrix_mode = MESSAGES;
  }
  else if (cmd.startsWith("DEL:"))
  {
    int idx = cmd.substring(4).toInt();
    if (idx >= 0 && idx < numMessages)
    {
      for (int i = idx; i < numMessages - 1; i++)
      {
        messages[i] = messages[i + 1];
      }
      numMessages--;
      msg = "Deleted!";
    }
    else
    {
      msg = "Invalid index!";
    }
    matrix_mode = MESSAGES;
  }
  else if (cmd.equalsIgnoreCase("LIST"))
  {
    msg = "Messages:";
    for (int i = 0; i < numMessages; i++)
    {

      msg = msg + i + ": " + messages[i];
    }
  }
  else if (cmd.startsWith("FONT_SMALL"))
  {
    matrix.setFont(&TomThumb);
    font_mode = SMALL;
  }
  else if (cmd.startsWith("FONT_BIG"))
  {
    matrix.setFont(nullptr);
    font_mode = BIG;
  }
  else
  {
    msg = "Unkown command.";
  }

  ble_driver->sendDataPacket(msg.c_str(), sizeof(msg.c_str()));
}


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
unsigned long last_ping = 0;
unsigned long wait_time = 5*60*1000;
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

      last_ping = millis();

    }else {
      if ((millis() - last_ping) > wait_time){
        String clear = "<CLEAR>";
        processCommand(clear);
      }
    }



  }
  updateDisplay();
}
