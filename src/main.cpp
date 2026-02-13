#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/TomThumb.h>
#include <SoftwareSerial.h>

// #include "state_machine.cpp"

#define DEBUG 0
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
#define MAX_MESSAGES 5
#define MAX_MESSAGE_SIZE 32
char messages[MAX_MESSAGES][MAX_MESSAGE_SIZE] = {
    "Hello world!",
    "NeoMatrix!",
    "Nano Rocks!"};

uint8_t numMessages = 3;

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
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum MATRIX_MODES
{
  PIXEL,
  MESSAGES,
  IDLE
};

enum FONT_MODES
{
  SMALL,
  BIG
};
FONT_MODES font_mode = SMALL;

MATRIX_MODES matrix_mode = MESSAGES;

void clearMatrix()
{
  DEBUG_PRINTLN("clear");
  matrix.clear();
  matrix.show();
  matrix_mode = IDLE;
}

void rainbow(char *cmd, uint8_t size)
{

  /**
   * Packet format should be in csv format:
   * p:x,y,r,g,b
   */
  DEBUG_PRINT("Pixel: ");

  if (size < 6)
  {
    // triggers if the packet is too small
    DEBUG_PRINT("false sized pixel_packet: ");
    DEBUG_PRINTLN(size);
    return;
  }

  matrix_mode = PIXEL;

  Pixel p;

  p.x = (int16_t)cmd[1];

  p.y = (int16_t)cmd[2];

  p.r = (uint8_t)cmd[3];

  p.g = (uint8_t)cmd[4];

  p.b = (uint8_t)cmd[5];

  DEBUG_PRINT(p.x);
  DEBUG_PRINT(",");
  DEBUG_PRINT(p.y);
  DEBUG_PRINT(",");
  DEBUG_PRINT(p.r);
  DEBUG_PRINT(",");
  DEBUG_PRINT(p.g);
  DEBUG_PRINT(",");
  DEBUG_PRINT(p.b);
  DEBUG_PRINTLN();

  matrix.drawPixel(p.x, p.y, matrix.Color(p.r, p.g, p.b));
  matrix.show();
}

void add(char *cmd, uint8_t size)
{
  DEBUG_PRINTLN("add");

  if (numMessages >= MAX_MESSAGES)
  {
    DEBUG_PRINTLN("Max messages reached");
    return;
  }

  // cmd[0] is the command '2', skip it
  uint8_t copySize = size - 1;
  if (copySize > (MAX_MESSAGE_SIZE - 1))
    copySize = MAX_MESSAGE_SIZE - 1;

  strncpy(messages[numMessages], cmd + 1, copySize);
  messages[numMessages][copySize] = '\0';
  numMessages++;

  matrix_mode = MESSAGES;
}

void del_msg(char *cmd, uint8_t size)
{
  DEBUG_PRINTLN("del");
  int idx = (uint8_t)cmd[1] - '0'; // ascii 0 to integer 0

  if (idx > 0 || idx >= numMessages){
    DEBUG_PRINT("Invalid Del Index: "); DEBUG_PRINTLN(idx);
    return;
  }

  for (uint8_t i = idx; i < numMessages - 1; i++)
  {
    strncpy(messages[i], messages[i + 1], MAX_MESSAGE_SIZE);
  }

  numMessages--;
  matrix_mode = MESSAGES;
}

void change_font(char *cmd, uint8_t size)
{
  switch (cmd[1])
  {
  case '1':
    DEBUG_PRINTLN("f_small");
    matrix.setFont(&TomThumb);
    font_mode = SMALL;
    break;

  case '2':
    matrix.setFont(nullptr);
    font_mode = BIG;
    break;

  default:
    break;
  }
}

char oki[] = "oki";
// Process Bluetooth command
void processCommand(char *cmd, uint8_t size)
{
  // String debug_msg = "Received command: " + cmd;
  // DEBUG_PRINTLN(debug_msg);
  // cmd.trim();

  switch ((uint8_t)cmd[0])
  {
  case '0':
    clearMatrix();
    break;

  case '1':
    rainbow(cmd, size);

    ble_driver->sendDataPacket(oki, sizeof(oki));
    break;

  case '2':
    add(cmd, size);
    ble_driver->sendDataPacket(oki, sizeof(oki));
    break;
  case '3':
    del_msg(cmd, size);
    ble_driver->sendDataPacket(oki, sizeof(oki));
    break;

  case '4':
    change_font(cmd, size);
    ble_driver->sendDataPacket(oki, sizeof(oki));
    break;

  default:
    break;
  }
}

// Update the matrix display
void updateDisplay()
{
  uint32_t now = millis();

  if (now - scrollTimer > scrollSpeed && matrix_mode == MESSAGES)
  {
    scrollTimer = now;

    matrix.fillScreen(0);

    int16_t x1, y1;
    uint16_t textWidth, h;
    matrix.getTextBounds(messages[currentMessage], 0, 0, &x1, &y1, &textWidth, &h);

    // 2. Determine vertical alignment based on font type
    int16_t cursorY = 0;
    
    switch (font_mode) {
      case SMALL:
        // TomThumb draws UP from the baseline. 
        // For an 8px high matrix, y=6 puts the text nicely at the bottom.
        cursorY = 6; 
        break;
        
      case BIG:
        // Default font draws DOWN from top-left.
        // y=0 puts it at the top.
        // y=1 centers it slightly better on an 8px high matrix.
        cursorY = 0; 
        break;
    }

    // 3. Set the cursor using the calculated Y and current scroll X
    matrix.setCursor(scrollX, cursorY); 

    matrix.print(messages[currentMessage]);
    matrix.show();

    scrollX--;

    if (scrollX + (int16_t)textWidth < 0)
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

void stripBrackets(char *msg, uint8_t &length)
{
  if (length < 2)
    return; // too short to have brackets
  if (msg[0] == '<' && msg[length - 1] == '>')
  {
    // Shift string left by 1 to remove '<'
    for (uint8_t i = 0; i < length - 2; i++)
    {
      msg[i] = msg[i + 1];
    }
    // Null terminate after removing '>'
    msg[length - 2] = '\0';
    length -= 2; // new length of the string
  }
}

/**
 * valid packets should look like this:
 * <programm_code(1byte)data>;
 */
boolean validPacket(char *buffer, uint8_t size)
{
  if (!buffer)
    return false;

  if (buffer[0] != '<')
    return false;

  // skip the first byte since we validated it.
  for (uint8_t i = 1; i < size; i++)
  {
    if (buffer[i] == '>')
      return true;

    if (buffer[i] == '\0')
      break;
  }

  return false;
}

#define RX_BUFFER_SIZE 32
char rxBuffer[RX_BUFFER_SIZE];
uint8_t rxIndex = 0;

void handleIncomingByte(char b)
{
  if (rxIndex == 0 && b != '<')
  {
    // ignore bytes until we get a '<'
    return;
  }

  if (rxIndex >= RX_BUFFER_SIZE)
  {
    // buffer full, reset
    rxIndex = 0;
    return;
  }

  rxBuffer[rxIndex++] = b;

  if (b == '>')
  {
    // complete packet received
    stripBrackets(rxBuffer, rxIndex);
    processCommand(rxBuffer, rxIndex);
    rxIndex = 0; // ready for next packet
  }
}

void setup()
{
  DEBUG_BEGIN(115200);

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

unsigned long last_ping = 0;
constexpr unsigned long wait_time =(unsigned long) 3 * (unsigned long)60 * (unsigned long)1000;
unsigned long time_delta = 0;
void loop()
{
  if (ble_driver->connected())
  {
    if (ble_driver->available())
    {
      char buffer[64];
      uint8_t received = ble_driver->get_received(buffer, sizeof(buffer));

      for (uint8_t i = 0; i < received; i++)
      {
        handleIncomingByte(buffer[i]);
      }
      last_ping = millis();
    }
  }
  DEBUG_PRINT(wait_time - (millis()-last_ping));
  DEBUG_PRINT("\t");
  DEBUG_PRINTLN(wait_time );
  if ((millis() - last_ping) > wait_time)
  {
    if (matrix_mode != IDLE)
    {
      char clear[] = "0";
      processCommand(clear, 1);
      clearMatrix();
      matrix_mode = IDLE;
      DEBUG_PRINTLN("IDLING");
    }
    else
    {
      delay(1000);
    }
  }
  else
  {
    if ((millis() - time_delta) > 1000)
    {
      DEBUG_PRINT("ping: ");
      DEBUG_PRINTLN(millis() - last_ping);
      time_delta = millis();
    }
    updateDisplay();
  }
}
