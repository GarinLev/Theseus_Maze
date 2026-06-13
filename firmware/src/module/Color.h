#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

struct HSVColor {
    float h;
    float s;
    float v;
    uint16_t c;
};

constexpr HSVColor TARGET_WHITE  = {172.30f, 0.13f, 0.285f, 1170};
constexpr HSVColor TARGET_BLUE   = {162.79f, 0.43f, 0.14f,  519};
constexpr HSVColor TARGET_BLACK = {133.82f, 0.57f, 0.10f, 373};
constexpr HSVColor TARGET_SILVER = {139.76f, 0.11f, 0.225f, 964};

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

    ColorType get_current_color(float threshold = 80.0f) const;

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    const float Rf = 1.09f;
    const float Gf = 0.78f;
    const float Bf = 1.24f;

    void hsv();

    uint16_t r{0}, g{0}, b{0}, c{0};
    float h{0.0f}, s{0.0f}, v{0.0f};

    float last_probabilities[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

#endif //FIRMWARE_COLOR_H