#include "Arduino.h"

class Marbles {
    private:
    struct Vector2D
    {
        uint8_t x;
        uint8_t y;
    };
    
    struct Marble
    {
        Vector2D position;
        Vector2D velocity;
    };

    public:
    Marbles(uint8_t num_marbles);
    tick();
    render();
    
}