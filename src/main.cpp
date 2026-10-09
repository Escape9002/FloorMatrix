#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

//--------------------------------------------------------
// HAL
//--------------------------------------------------------

#define MATRIX_PIN 12

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
    32, 8, MATRIX_PIN,
    NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

// ------------------------------------ MATRIX OUTPUT DEFINITIONS
#include "NeoMatrixEngine.h"
NeoMatrixEngine light_engine = NeoMatrixEngine(matrix);

#include "SerialLightEngine.h"
// SerialLightEngine light_engine = SerialLightEngine(Serial);

// ------------------------------------ BOARD HAL
#include "Nano33BLESense.h"
Nano33BLESense nano33ble_sense = Nano33BLESense();

// ------------------------------------ Light Sensor
#include "LightSensor.h"
LightSensor light_sensor = LightSensor();

// ------------------------------------ Microphone
#include "SoundSensor.h"
SoundSensor microphone = SoundSensor();

// ------------------------------------ TempHumSensor
#include "TempHumSensor.h"
TempHumSensor temp_hum_sensor = TempHumSensor();

//--------------------------------------------------------
// STATE MACHINE
//--------------------------------------------------------
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

  microphone.begin();

  light_sensor.begin();

  temp_hum_sensor.begin();
}

uint32_t last_tick_time = 0;
uint32_t last_frame_time = 0;
uint32_t last_temp_read_time = 0;

constexpr uint32_t FRAME_INTERVAL_MS = 100; // ~30 FPS
constexpr uint32_t TEMP_INTERVAL_MS  = 2000; // Read I2C sensor once every 2 seconds
float cached_temp = 20.0f;
float cached_hum = 50.0f;
void loop()
{
  uint32_t now = millis();
  uint32_t dt = now - last_tick_time;
  last_tick_time = now;

  uint16_t dB = microphone.read_decibels();
  uint16_t brightness = light_sensor.read_lux();

    if (now - last_temp_read_time >= TEMP_INTERVAL_MS)
    {
        last_temp_read_time = now;
        cached_temp = temp_hum_sensor.get_temp();
        cached_hum = temp_hum_sensor.get_hum();
    }
  // if (dB > 75){
  //   delay(500); // manual debounce
  // }

  Serial.print((dB > 75));
  Serial.print("\t");
  Serial.print(matrix_state_engine.get_state());
  Serial.print("\t");
  Serial.println(brightness);
  
  
  matrix_state_engine.tick(dB, brightness, dt, cached_temp, cached_hum);

  // 2. Refresh display at a stable, throttled rate
  if (now - last_frame_time >= FRAME_INTERVAL_MS)
  {
    matrix_state_engine.render();
    last_frame_time = now;
  }
}
