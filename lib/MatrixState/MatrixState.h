#include "states/TimeState.h"
#include "HAL.h"
#include "LightEngine.h"

#include <cstdint>
#include "states/TemperatureState.h"
enum MATRIX_STATE : uint8_t
{
    OFF,
    TIME,
    TEMPHUM
};

class MatrixState
{

protected:
    HAL &hal;
    LightEngine &light_engine;
    MATRIX_STATE state;
    uint8_t wanted_brightness;

    TimeState time_state;
    TemperatureState temp_state;

    // LED Matrix used in the evening, thus AVG_ROOM_LUX is
    // ~ 500 lux and the Matrix should be calibrated against that.
    static constexpr uint16_t AVG_ROOM_LUX = 500;

    // wanted brightness to use when reaching AVG_ROOM_LUX
    static constexpr uint16_t SCALED_BRIGHTNESS = 125;

    // https://www.geo.de/wissen/forschung-und-technik/physik-des-klatschens--jeder-mensch-erzeugt-einen-einzigartigen-klang-35777172.html
    // loudest clapping is around 85dB, 60-70 dB is talking/loud music.
    // so we go with 75, dB is a exponential scale!
    static constexpr uint16_t CLAP_DB = 85;

    uint32_t clap_cooldown_ms = 0;

private:
    // The States of the Matrix should not depend on anything.
    // their construction should thus be simple / not need a constructor.

    // TimeState requires the start-time to be set correctly.
    // It will be handed in via the BLE interface.
    TimeState timeState;
    TemperatureState tempState;

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
    MatrixState(HAL &hal, LightEngine &light_engine) : hal(hal), light_engine(light_engine), state(MATRIX_STATE::TIME), wanted_brightness(20)
    {
    }

    /**
     * @brief call the current states tick() function.
     *
     */
    void tick(uint16_t sound, uint16_t lux, uint32_t dt_ms, float temp, float hum)
    {

        if (clap_cooldown_ms > 0)
        {
            if (dt_ms >= clap_cooldown_ms)
                clap_cooldown_ms = 0;
            else
                clap_cooldown_ms -= dt_ms;
        }

        // Toggle state only if above threshold and cooldown has expired
        if (clap_cooldown_ms == 0 && sound > CLAP_DB)
        {
            clap_cooldown_ms = 600; // Ignore further sound spikes for 600ms

            switch (state)
            {
            case MATRIX_STATE::OFF:
                
                state = MATRIX_STATE::TIME;
                break;
            case MATRIX_STATE::TIME:
                
                state = MATRIX_STATE::TEMPHUM;
                break;
            case MATRIX_STATE::TEMPHUM:
                
                state = MATRIX_STATE::OFF;
                break;
            }
        }

        wanted_brightness = brightness_input(lux);

        time_state.tick(dt_ms);
        temp_state.tick(dt_ms);

        tempState.set_humidity(hum);
        tempState.set_temperature(temp);
    }

    /**
     * @brief
     *
     */
    void render()
    {

        switch (state)
        {
        case MATRIX_STATE::OFF:
            light_engine.off();
            break;

        case MATRIX_STATE::TIME:
        {
            const char *time_text = time_state.get_time_str();
            light_engine.setText(time_text);
            break;
        }
        case MATRIX_STATE::TEMPHUM:
        {
            const char *temp_text = tempState.get_temp_hum_str();
            light_engine.setText(temp_text);
            break;
        }
        }

        light_engine.setBrightness(wanted_brightness);
        light_engine.show();
    }

    MATRIX_STATE get_state()
    {
        return state;
    }
};