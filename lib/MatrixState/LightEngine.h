#pragma once
#include <cstdint>

class LightEngine
{
public:
    // Virtual destructor is required for interfaces
    virtual ~LightEngine() = default;

    // The effects your controller can ask for
    virtual void off() = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void setText(const char* msg) = 0;
    virtual void show() = 0;
};