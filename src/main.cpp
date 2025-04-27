#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 12
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);


uint16_t buildRGB565(uint8_t red, uint8_t green, uint8_t blue){
    uint16_t red_565 = map(red, 0, 255, 0, 31);

    uint16_t green_565 = map(green, 0, 255, 0, 63);
    
    uint16_t blue_565 = map(blue, 0, 255, 0, 31);

    return (red_565<<11)|(green_565<<5)|blue_565;
}

struct Vector2D
{
    uint8_t x;
    uint8_t y;
};

struct Particle
{
    Vector2D position;
    Vector2D velocity;
};

#define NUM_PARTICLES 1
Particle particles[NUM_PARTICLES];
Vector2D const_force = {10,10};
bool matrix[MATRIX_WIDTH][MATRIX_HEIGHT] = {false};

void tick(){
    for (int i = 0; i < NUM_PARTICLES; i ++){
        uint8_t newX = particles[i].position.x + particles[i].velocity.x;
        uint8_t newY = particles[i].position.y + particles[i].velocity.y;

        if(newX < matrix.width() && newX >= 0){
            particles[i].position.x = newX;
        }

        if( newY < matrix.height() && newY >= 0){
            particles[i].position.y = newY;
        }

        

    }
}

void render(){
    matrix.clear();
    for (int i = 0; i < NUM_PARTICLES; i ++){
        matrix.drawPixel(particles[i].position.x,particles[i].position.y,buildRGB565(0,0,255));
        Serial.print(particles[i].position.x);
        Serial.print("\t");
        Serial.println(particles[i].position.y);
    }
    matrix.show();
}

  

void setup()
{
  

  Serial.begin(9600);
    matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(20);
  matrix.fillScreen(0);

  
  for (int i = 0; i < NUM_PARTICLES; i ++){
    particles[i].position ={0,0};
    particles[i].velocity = {1,0};
    
  }

}

int blue = 0;
void loop()
{

tick();
render();
delay(150);
    // matrix.clear();
          
    // matrix.drawPixel(15,0,buildRGB565(100,100,100));
    // matrix.drawPixel(20,4,buildRGB565(0,255,0));
    // matrix.drawPixel(30,3,buildRGB565(0,0,255));

    // matrix.show();

  
}