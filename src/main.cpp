#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Pin definitions
#define MATRIX_PIN 12

// Create objects
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
    32, 8, MATRIX_PIN,
    NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

#include "NeoMatrixEngine.h"
// NeoMatrixEngine light_engine = NeoMatrixEngine(matrix);

#include "SerialLightEngine.h"
SerialLightEngine light_engine = SerialLightEngine(Serial);


#include "Nano33BLESense.h"
Nano33BLESense nano33ble_sense = Nano33BLESense();

#include "MatrixState.h"
MatrixState matrix_state_engine = MatrixState(nano33ble_sense, light_engine);

void setup()
{
Serial.begin(115200);
   // On native USB boards like the Nano 33 BLE, wait briefly for 
    // the Serial Monitor to connect so you don't miss the initial frames
    while (!Serial && millis() < 3000)
    {
        // Wait up to 3 seconds, then continue anyway
    }

    Serial.println(F("--- Clock Console Simulation Started ---"));
  matrix.begin();

  matrix.setBrightness(5);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  matrix.show();
}

uint32_t last_tick_time = 0;
uint32_t last_frame_time = 0;
constexpr uint32_t FRAME_INTERVAL_MS = 1000; // ~30 FPS

void loop()
{
  uint32_t now = millis();
  uint32_t dt = now - last_tick_time;
  last_tick_time = now;

  matrix_state_engine.tick(0,500 , dt);
  
  // 2. Refresh display at a stable, throttled rate
  if (now - last_frame_time >= FRAME_INTERVAL_MS)
  {
    matrix_state_engine.render();
    last_frame_time = now;
    
  }
}
