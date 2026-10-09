
#include <Arduino.h>
#include <PDM.h>
#include <cmath>

class SoundSensor
{
public:
    static constexpr int SAMPLE_COUNT = 256; // ~16ms window at 16kHz

private:
    static inline short sample_buffer[SAMPLE_COUNT];
    static inline volatile bool data_ready = false;

    // Interrupt service routine callback from the PDM library
    static void on_pdm_data()
    {
        int bytes_available = PDM.available();
        PDM.read(sample_buffer, bytes_available);
        data_ready = true;
    }

public:
    bool begin()
    {
        PDM.onReceive(on_pdm_data);
        
        // 1 channel (mono), 16000 Hz sample rate
        if (!PDM.begin(1, 16000))
        {
            return false;
        }

        // Optional: PDM gain (0 to 80, default is ~20 on nRF52)
        PDM.setGain(20);
        return true;
    }

    /**
     * @brief Computes current Sound Pressure Level in dB.
     * @return uint16_t dB value (e.g. 40-100 dB), or 0 if no new samples.
     */
    uint16_t read_decibels()
    {
        if (!data_ready)
        {
            return 0;
        }

        data_ready = false;

        // 1. Calculate sum of squares
        uint64_t sum_squares = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++)
        {
            int32_t sample = sample_buffer[i];
            sum_squares += (sample * sample);
        }

        // 2. Compute RMS
        double rms = std::sqrt(static_cast<double>(sum_squares) / SAMPLE_COUNT);

        if (rms < 1.0)
        {
            return 30; // Ambient noise floor clamp (~30 dB)
        }

        // 3. Convert RMS to approximate dBSPL
        // dB = 20 * log10(rms / 32767.0) + 120.0
        double db = (20.0 * std::log10(rms / 32767.0)) + 120.0;

        if (db < 0.0)   db = 0.0;
        if (db > 120.0) db = 120.0;

        return static_cast<uint16_t>(db);
    }
};