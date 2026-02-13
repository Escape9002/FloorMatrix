#if defined(USE_HM10_DRIVER)
// Debug macros are best placed here or in the header, outside the class
#define DEBUG 0
#if DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif

#include "HM10.h"
#include <Arduino.h>
// #include <AltSoftSerial.h>

////////////////////////////////////// MOVE TO .h if possible
// AltSoftSerial btSerial;
#define btSerial Serial
//////////////////////////////////////

// --- Static Member Initialization ---
HM10 *HM10::_instance = nullptr;

// --- Singleton Accessor ---
HM10 &HM10::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new HM10();
    }
    return *_instance;
}

HM10::HM10()
{
    // nothing to do here
}

bool HM10::begin()
{
    _instance = this; // Set the instance pointer for static callbacks

    DEBUG_PRINTLN("Configuring Bluetooth...");
    btSerial.begin(115200);

    // have a look at the documenation (the pdf)
    // for info on what the commands do.

    if (!sendATCommand("AT"))
        return false; // Test communication

    if (!sendATCommand("AT+IMME1"))
        return false; // Test communication

    if (!sendATCommand("AT+POWE3"))
        return false;

    if (!sendATCommand("AT+RESET"))
        return false; // Soft reset

    if (!sendATCommand("AT+NAMENeoMatrix"))
        return false; // Set device name

    if (!sendATCommand("AT+BAUD4"))
        return false; // Set baud rate to 9600

    if (!sendATCommand("AT+ROLE0"))
        return false; // Set to slave role

    if (!sendATCommand("AT+COMA0"))
        return false; // Set to slave role
    
    if (!sendATCommand("AT+COMI0"))
        return false; // Set to slave role
    
    if (!sendATCommand("AT+GAIN1"))
        return false; // Set to slave role

    if (!sendATCommand("AT+PCTL1"))
        return false; // Set to slave role


    if (!sendATCommand("AT+START"))
        return false; // Set to slave role

    return true;
}

bool HM10::sendATCommand(const char *cmd)
{

    btSerial.flush();

    btSerial.print(cmd);

    // if you want to see more of the HM-10-Response,
    // increase the array size.
    char response[3];
    int idx = 0;

    unsigned long start = millis();

    // some commands take a full second till the HM10 answers.
    // give it plenty of time
    while (millis() - start < 1000)
    {
        while (btSerial.available())
        {
            char c = btSerial.read();

            if (idx < sizeof(response) - 1)
                response[idx++] = c;
        }
    }

    response[idx] = '\0';

    DEBUG_PRINT("CMD: ");
    DEBUG_PRINT(cmd);
    DEBUG_PRINT("\tHM-10 Response: ");
    DEBUG_PRINTLN(response);

    return strstr(response, "OK") != nullptr;
}

bool HM10::connected()
{
    return btSerial.available();
}

bool HM10::sendDataPacket(const void *data, uint16_t len)
{
    // Cast the void pointer to an unsigned char array
    const unsigned char *buffer = reinterpret_cast<const unsigned char *>(data);

    // Write the data packet to the serial interface (btSerial.write())
    btSerial.write(buffer, len);

    return true; // Assuming successful write operation
}

bool HM10::sendWhenReady(const void *data, uint16_t len)
{
    // TODO implement batched sending
    return false;
}

bool HM10::available()
{
    return btSerial.available();
}

String HM10::get_received()
{
    String inputBuffer = "";

    while (btSerial.available())
    {
        char c = btSerial.read();
        inputBuffer += c;
    }
    inputBuffer.trim();

    return inputBuffer;
}

uint8_t HM10::get_received(char *buffer, uint8_t size)
{
    uint8_t idx = 0;
    while (btSerial.available() && idx < size)
    {
        char c = btSerial.read();

        buffer[idx++] = c;
    }
    return idx;
}

#endif