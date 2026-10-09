#pragma once
#include <cstdio>
#include <cstdint>

class TemperatureState
{
private:
    float temperature = 0.0f;
    float humidity = 0.0f;

    uint32_t toggle_timer_ms = 0;
    bool show_humidity = false;

    char formatted_str[8] = " 00 C";

    void update_string()
    {
        if (show_humidity)
        {
            // "%2.0f %%" -> e.g. " 48 %" (escaped %% for literal percent)
            snprintf(formatted_str, sizeof(formatted_str), "%2.0f %%", humidity);
        }
        else
        {
            // "%2.0f C" -> e.g. " 23 C"
            snprintf(formatted_str, sizeof(formatted_str), "%2.0f C", temperature);
        }
    }

public:
    void tick(uint32_t dt_ms)
    {
        toggle_timer_ms += dt_ms;
        // Swap between temperature and humidity every 2.5 seconds
        if (toggle_timer_ms >= 2500)
        {
            toggle_timer_ms = 0;
            show_humidity = !show_humidity;
        }
    }

    void set_temperature(float temp)
    {
        temperature = temp;
    }

    void set_humidity(float hum)
    {
        humidity = hum;
    }

    const char *get_temp_hum_str()
    {
        update_string();
        return formatted_str;
    }
};