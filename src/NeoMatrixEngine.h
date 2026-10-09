#pragma once
#include "LightEngine.h"

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

class NeoMatrixEngine : public LightEngine
{
private:

    Adafruit_NeoMatrix &matrix;
  
public:
    NeoMatrixEngine(Adafruit_NeoMatrix &matrix) : matrix(matrix)
    {
    }

    void off() override
    {
        
        matrix.fillScreen(0);
        
    }

    void setBrightness(uint8_t brightness) override {
        // remove this safeguard if the powersupply can keep up again.
        // only temporary :)
        if(brightness > 5) {
            brightness  = 5;
        }
        matrix.setBrightness(brightness);
    }

    void setText(const char* msg) override {
        matrix.setCursor(0, 0); 
        matrix.print(msg);
    }

    void show() override {
        matrix.show();
    }
};