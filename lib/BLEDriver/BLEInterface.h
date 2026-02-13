#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H

#include "Arduino.h"

class BLEInterface {
public:
    virtual ~BLEInterface() {} // Virtual destructor is important for interfaces

    virtual bool begin() = 0;
    virtual bool connected() = 0;
    virtual bool sendDataPacket(const void* data, uint16_t len) = 0;
    virtual bool sendWhenReady(const void* data, uint16_t len) = 0;
    virtual bool available() = 0;
    virtual String get_received() = 0;
    virtual uint8_t get_received(char *buffer, uint8_t size) = 0;
};

#endif // BLE_INTERFACE_H