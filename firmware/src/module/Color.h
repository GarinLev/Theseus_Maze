#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

class Color {
public:
    void init();
    void update();
    void calibrate_color();
private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);
    uint16_t last_update_ms;

    void hsv();

    uint16_t r, g, b, c;
    float h, s, v;
};


#endif //FIRMWARE_COLOR_H
