#pragma once

#include <cstdint>
#include <cstdio>
#include <cctype>

class TimeState
{
private:
    uint32_t time_ms = 0;
    char formatted_time[6] = "00:00"; // 5 characters + '\0'

    void update_string()
    {
        uint32_t total_sec = time_ms / 1000;
        uint32_t total_min = total_sec / 60;

        uint8_t hours = (total_min / 60) % 24;
        uint8_t minutes = total_min % 60;

        // "%02u" formats as unsigned int padded with a leading zero if < 10
        snprintf(formatted_time, sizeof(formatted_time), "%02u:%02u", hours, minutes);
    }

public:
    static constexpr uint32_t MS_PER_DAY = 86400000UL; // 24 * 60 * 60 * 1000
    TimeState() = default;

    /**
     * @brief Set the time directly in milliseconds.
     */
    void set_time_ms(uint32_t ms)
    {
        time_ms = ms % MS_PER_DAY;
        
    }

    /**
     * @brief Set the time from a formatted string: "HH:MM" (e.g. "07:05", "23:42").
     * Validates input characters and bounds (00-23 hours, 00-59 minutes).
     *
     * @return true if successfully parsed, false if format/range is invalid.
     */
    bool set_time(const char *str)
    {
        if (str == nullptr)
            return false;

        // Check if digits and colon are present at expected positions
        if (!isdigit(str[0]) || !isdigit(str[1]) || str[2] != ':' ||
            !isdigit(str[3]) || !isdigit(str[4]))
        {
            return false;
        }

        uint8_t hours = (str[0] - '0') * 10 + (str[1] - '0');
        uint8_t minutes = (str[3] - '0') * 10 + (str[4] - '0');

        if (hours > 23 || minutes > 59)
        {
            return false;
        }

        time_ms = (static_cast<uint32_t>(hours) * 3600UL +
                   static_cast<uint32_t>(minutes) * 60UL) *
                  1000UL;

        
        return true;
    }

    /**
     * @brief Advance the internal clock by dt_ms and return the current time string.
     *
     * @param dt_ms Delta time in milliseconds since the last tick.
     * @return const char* Pointer to the internal null-terminated "HH:MM" buffer.
     */
    void tick(uint32_t dt_ms)
    {
        time_ms = (time_ms + dt_ms) % MS_PER_DAY;
        
    }

    /**
     * @brief Get the time string without advancing time.
     * Only compute time-string if necessary
     */
    const char *get_time_str()
    {
        update_string();
        return formatted_time;
    }
};