#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstring>
#include "LightEngine.h"

class SerialLightEngine : public LightEngine
{
public:
    static constexpr uint8_t WIDTH  = 32;
    static constexpr uint8_t HEIGHT = 8;

private:
    Print& out;
    uint8_t brightness = 255;
    bool is_on = true;
    char current_text[16] = {0};
    bool grid[HEIGHT][WIDTH] = {{false}};

    // Compact 5x7 font bitmaps for digits, colon, and common chars
    // Each row is 5 bits wide (bit 4 = left, bit 0 = right)
    static const uint8_t* get_glyph(char c)
    {
        static const uint8_t FONT_0[7] = {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
        static const uint8_t FONT_1[7] = {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        static const uint8_t FONT_2[7] = {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111};
        static const uint8_t FONT_3[7] = {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110};
        static const uint8_t FONT_4[7] = {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010};
        static const uint8_t FONT_5[7] = {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110};
        static const uint8_t FONT_6[7] = {0b01110, 0b10001, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
        static const uint8_t FONT_7[7] = {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
        static const uint8_t FONT_8[7] = {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
        static const uint8_t FONT_9[7] = {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b10001, 0b01110};
        static const uint8_t FONT_COLON[7] = {0b00000, 0b00100, 0b00000, 0b00000, 0b00000, 0b00100, 0b00000};
        static const uint8_t FONT_DASH[7]  = {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000};
        static const uint8_t FONT_SPACE[7] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
        static const uint8_t FONT_BOX[7]   = {0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111};

        switch (c)
        {
        case '0': return FONT_0;
        case '1': return FONT_1;
        case '2': return FONT_2;
        case '3': return FONT_3;
        case '4': return FONT_4;
        case '5': return FONT_5;
        case '6': return FONT_6;
        case '7': return FONT_7;
        case '8': return FONT_8;
        case '9': return FONT_9;
        case ':': return FONT_COLON;
        case '-': return FONT_DASH;
        case ' ': return FONT_SPACE;
        default:  return FONT_BOX; // Fallback for unknown characters
        }
    }

    void rasterize()
    {
        // Clear frame buffer
        memset(grid, 0, sizeof(grid));

        if (!is_on || current_text[0] == '\0') {
            return;
        }

        // Measure text length
        size_t len = 0;
        while (current_text[len] != '\0' && len < sizeof(current_text) - 1) {
            len++;
        }

        // 5 pixels wide + 1 pixel spacing between chars
        int total_width = (len > 0) ? static_cast<int>(len * 6 - 1) : 0;
        int start_x = (WIDTH - total_width) / 2;
        if (start_x < 0) start_x = 0;

        int current_x = start_x;
        int start_y = (HEIGHT - 7) / 2; // Center 7px font vertically in 8px height

        for (size_t i = 0; i < len; ++i)
        {
            const uint8_t* glyph = get_glyph(current_text[i]);

            for (int row = 0; row < 7; ++row)
            {
                int y = start_y + row;
                if (y < 0 || y >= HEIGHT) continue;

                uint8_t row_bits = glyph[row];
                for (int col = 0; col < 5; ++col)
                {
                    int x = current_x + col;
                    if (x < 0 || x >= WIDTH) continue;

                    // Bit 4 is leftmost pixel, Bit 0 is rightmost
                    grid[y][x] = ((row_bits >> (4 - col)) & 0x01);
                }
            }
            current_x += 6; // 5 columns + 1 spacing
            if (current_x >= WIDTH) break;
        }
    }

public:
    explicit SerialLightEngine(Print& output = Serial)
        : out(output)
    {
    }

    void off() override
    {
        is_on = false;
    }

    void setBrightness(uint8_t b) override
    {
        brightness = b;
    }

    void setText(const char* msg) override
    {
        if (msg == nullptr)
        {
            current_text[0] = '\0';
            return;
        }

        size_t i = 0;
        while (msg[i] != '\0' && i < sizeof(current_text) - 1)
        {
            current_text[i] = msg[i];
            i++;
        }
        current_text[i] = '\0';
        is_on = true;
    }

    void show() override
    {
        rasterize();

        // Top border with status metadata
        out.print(F("+"));
        for (uint8_t c = 0; c < WIDTH; ++c) out.print(F("-"));
        out.print(F("+ [BRIGHTNESS: "));
        out.print(brightness);
        out.print(is_on ? F(" | ON") : F(" | OFF"));
        out.println(F("]"));

        // Render rows
        for (uint8_t r = 0; r < HEIGHT; ++r)
        {
            out.print(F("|"));
            for (uint8_t c = 0; c < WIDTH; ++c)
            {
                out.print(grid[r][c] ? '#' : ' ');
            }
            out.println(F("|"));
        }

        // Bottom border
        out.print(F("+"));
        for (uint8_t c = 0; c < WIDTH; ++c) out.print(F("-"));
        out.println(F("+"));
        out.println(); // Blank line separator between frames
    }
};