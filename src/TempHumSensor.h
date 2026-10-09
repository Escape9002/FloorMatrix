#pragma once

#include <Arduino_HTS221.h>

class TempHumSensor
{

public:
    TempHumSensor()
    {
    }

    void begin()
    {
        if (!HTS.begin())
        {
            Serial.println("Failed to initialize humidity temperature sensor!");
        }
    }

    float get_temp()
    {
        return HTS.readTemperature();
    }

    float get_hum()
    {
        return HTS.readHumidity();
    }
};