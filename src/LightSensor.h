#pragma once

#include <Arduino.h>
#include <Arduino_APDS9960.h>

class LightSensor
{
private:
    uint16_t last_lux = 500; // Default to average room lux until first read

public:
    bool begin()
    {
        if (!APDS.begin())
        {
            return false;
        }
        return true;
    }

    /**
     * @brief Reads the current ambient light level non-blockingly.
     * @return uint16_t Ambient light level (Clear channel ADC count).
     */
    uint16_t read_lux()
    {
        // Only read when new hardware data is ready; otherwise return last cached value
        if (APDS.colorAvailable())
        {
            int r, g, b, a;
            APDS.readColor(r, g, b, a);
            
            if (a >= 0)
            {
                last_lux = static_cast<uint16_t>(a);
            }
        }

        return last_lux;
    }
};