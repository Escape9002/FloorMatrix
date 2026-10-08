#include <cstdint>
#include <cstdio>
class TimeState
{
private:
    unsigned long time = 0;

    /**
     * @brief Get the time as char
     *
     * @param time in ms
     * @return char[] "00:00";
     */
    char[] get_time_char(unsigned long time)
    {
        /**
         * @brief Convert from ms to hours and minutes
         * ms / 1000 = sec
         * sec / 60 = min
         * min / 60 = h
         *
         * Be carefull not to round too early, as
         * the hours/minutes/sec values might be off otherwise.
         * e.g. 120 min = 2h, but 120min % 60 = 1h
         */

        uint32_t compu_sec = time / 1000;
        uint32_t compu_min = sec / 60;
        uint32_t compu_h = min / 60;

        uint8_t min = compu_min % 60;
        uint8_t h = compu_h % 24;

        char msg[5];
        if(h < 9) {
            msg[0] = "0";
            msg[1] = h;
        }

        if(min < 9) {
            msg[3] = 0;
            msg[4] = min;
        }

        
        snprintf(msg, "{%d}:{$d}");

        return msg;
    }

public:
    void set_time(unsigned long time)
    {
        this->time = time;
    }

    char[] tick(unsigned long dt_ms)
    {
        time += dt_ms;

        char msg[] = get_time_char(time);
    }
}