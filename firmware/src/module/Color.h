#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

struct HSVColor {
    float h;
    float s;
    float v;
};

// Твои константы высокой точности
constexpr HSVColor TARGET_WHITE  = {116.1040f, 0.0764f, 0.0128f};
constexpr HSVColor TARGET_BLUE   = {178.0899f, 0.4214f, 0.0074f};
constexpr HSVColor TARGET_BLACK  = {142.9941f, 0.4367f, 0.0061f};
constexpr HSVColor TARGET_SILVER = {191.6118f, 0.1660f, 0.0128f};

enum ColorType : uint8_t {
    COLOR_WHITE = 0,
    COLOR_BLUE,
    COLOR_BLACK,
    COLOR_SILVER,
    COLOR_UNKNOWN = 255
};

class Color {
public:
    void init();
    void update();
    void calibrate_color();

    float match(HSVColor target) const;
    void compute(const HSVColor targets[], float outputs[], size_t count) const;

    ColorType get_current_color(float threshold = 85.0f) const;

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    const float Rf = 1.17f;
    const float Gf = 0.72f;
    const float Bf = 1.33f;

    void hsv();

    uint16_t r{0}, g{0}, b{0}, c{0};
    float h{0.0f}, s{0.0f}, v{0.0f};

    float last_probabilities[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

#endif //FIRMWARE_COLOR_H