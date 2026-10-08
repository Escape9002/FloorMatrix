#include <cstdint>
enum MATRIX_STATE
{
    OFF,
    TIME
};

enum BLE_TYPE
{
    TIME,
    STATE,
    MESSAGE
};

struct BLE_INPUT
{
    BLE_TYPE type;
    char msg[16];
};

class MatrixState
{

protected:
    HAL &hal;
    LightEngine &light_engine;
    MATRIX_STATE state;
    uint8_t wanted_brightness;

    // LED Matrix used in the evening, thus AVG_ROOM_LUX is
    // ~ 500 lux and the Matrix should be calibrated against that.
    static constexpr uint16_t AVG_ROOM_LUX = 500;

    // wanted brightness to use when reaching AVG_ROOM_LUX
    static constexpr uint16_t SCALED_BRIGHTNESS = 125;

    // https://www.geo.de/wissen/forschung-und-technik/physik-des-klatschens--jeder-mensch-erzeugt-einen-einzigartigen-klang-35777172.html
    // loudest clapping is around 85dB, 60-70 dB is talking/loud music.
    // so we go with 75, dB is a exponential scale!
    static constexpr uint16_t CLAP_DB = 75;

private:
    // The States of the Matrix should not depend on anything.
    // their construction should thus be simple / not need a constructor.

    // TimeState requires the start-time to be set correctly.
    // It will be handed in via the BLE interface.
    TimeState timeState;

    bool clap_detect(uint16_t dB)
    {
        return (dB > CLAP_DB);
    }

    /**
     * @brief Scale the measured brightness into 0-255 values.
     *
     * AVG = 500 -> wanted = 125
     * Upper = 1000 -> wanted = 255
     * Lower = 20 -> wanted = 10
     *
     * scale = (255 - 10) / (1000 - 20) = 245/980 = 0.25
     *
     *
     * brightness = 255 - (x / AVG
     *
     * @param lux
     * @return uint8_t
     */
    uint8_t brightness_input(uint16_t lux)
    {
        static const uint16_t FP_SCALE = 1024;

        static const uint16_t UPPER_LUX = 1000;
        static const uint16_t LOWER_LUX = 20;
        static const uint16_t UPPER_BRIGHT = 255;
        static const uint16_t LOWER_BRIGHT = 0;

        // the normal scale returns a float, which is why we scale everything with FP_SCALE to
        // get a valid int.
        // remember to divide it away again later.
        static const uint16_t LUX_SCALE = ((UPPER_LUX - LOWER_LUX) * FP_SCALE) / (UPPER_BRIGHT - LOWER_BRIGHT);

        return static_cast<uint8_t>((LUX_SCALE * lux) / FP_SCALE);
    }

public:
    MatrixState(HAL &hal, LightEngine &light_engine) : HAL(hal), LightEngine(light_engine)
    {
        state = MATRIX_STATE::ON;
        wanted_brightness = 100;

        timeState = TimeState();
    }

    /**
     * @brief call the current states tick() function.
     *
     */
    void tick(uint32_t dt_ms)
    {



        switch (state)
        {
        case MATRIX_STATE::OFF:
            light_engine.off();
            break;
        case MATRIX_STATE::TIME:
            char text[] = time_state.tick(dt_ms);
            light_engine.show(text);

        default:
            break;
        }

        light_engine.setBrightness(wanted_brightness);
    }

    /**
     * @brief evaluate all inputs and go to the next state if necessary.
     *
     */
    void render(uint16_t sound, BLE_INPUT ble_input, uint16_t lux)
    {
        bool clapped = clap_detect(sound);

        uint8_t brightness = brightness_input(lux);

        if (clapped)
        {
            if (state == MATRIX_STATE::TIME)
            {
                state = MATRIX_STATE::OFF;
            }
            else
            {
                state = MATRIX_STATE::TIME;
            }
        }

        wanted_brightness = brightness_input(lux);

        switch (ble_input.type)
        {
        case BLE_TYPE::TIME:
            // extract the time from the char array
            // the time will be formated as "xx:xx", 
            // but there are more chars in the array then these 5.
            // cut the array to the uniform size.
            char time = get_time_str(ble_input.msg);

            // extract the time in ms from the given uniform char array.
            unsigned long time_ms = get_time_ms(time);
            timeState.setTime(time_ms);
            break;

        default:
            // do nothing with the others for now
            break;
        }
    }
};