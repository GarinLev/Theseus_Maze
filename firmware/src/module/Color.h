#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

struct HSVColor {
    float h;
    float s;
    float c;
};

constexpr HSVColor TARGET_WHITE  = {41.17f,  0.01f, 0.97f};
constexpr HSVColor TARGET_BLUE   = {224.90f, 0.37f, 0.94f};
constexpr HSVColor TARGET_BLACK  = {341.70f, 0.25f, 0.03f};
constexpr HSVColor TARGET_SILVER = {15.00f,  0.10f, 1.00f};


enum ColorType : uint8_t {
    COLOR_WHITE = 0,
    COLOR_BLUE = 1,
    COLOR_BLACK = 2,
    COLOR_SILVER = 3,
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
    float read_normalized() const;

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    const float Rf = 0.91f;
    const float Gf = 0.96f;
    const float Bf = 1.16f;

    uint8_t pin_led = A1;
    float led_max = 410.0f;
    float led_min = 25.0f;

    void hsv();

    uint16_t r{0}, g{0}, b{0};
    float h{0.0f}, s{0.0f}, c{0.0f};

    float last_probabilities[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

#endif //FIRMWARE_COLOR_H