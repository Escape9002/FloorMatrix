
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