#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

struct HSVColor {
    float h;
    float s;
    float v;
    uint16_t c;
};

constexpr HSVColor TARGET_WHITE  = {349.30f, 0.25f, 0.289f, 1043};
constexpr HSVColor TARGET_BLUE   = {242.64f, 0.44f, 0.160f, 383};
constexpr HSVColor TARGET_BLACK  = {344.27f, 0.50f, 0.028f, 77};
constexpr HSVColor TARGET_SILVER = {339.89f, 0.22f, 0.340f, 1266};

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
    void log();

    float match(HSVColor target) const;
    void compute(const HSVColor targets[], float outputs[], size_t count) const;
    ColorType get_current_color(float threshold = 80.0f) const;

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    const float Rf = 1.04f;
    const float Gf = 0.82f;
    const float Bf = 1.22f;

    void hsv();

    uint16_t r{0}, g{0}, b{0}, c{0};
    float h{0.0f}, s{0.0f}, v{0.0f};

    float last_probabilities[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

#endif //FIRMWARE_COLOR_H