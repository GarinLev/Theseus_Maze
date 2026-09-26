#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Adafruit_TCS34725.h>

struct HSVColor {
    float h;
    float s;
    float c;
};

constexpr HSVColor TARGET_WHITE  = {0.00, 0.07, 1.00};
constexpr HSVColor TARGET_SILVER = {0.00, 0.23, 1.00};
constexpr HSVColor TARGET_BLACK  = {356.01, 0.46, 0.00};
constexpr HSVColor TARGET_BLUE   = {225.44, 0.59, 0.28};

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
    void log();

    float distance_to(HSVColor target) const;
    ColorType get_current_color(float max_distance = 0.40f) const;
    float read_normalized();

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    const float Rf = 0.85f;
    const float Gf = 0.91f;
    const float Bf = 1.40f;

    uint8_t pin_led = A1;
    uint16_t raw_led = 0; // Сырое значение analogRead
    float led_max = 670.0f;
    float led_min = 620.0f;

    void hsv();

    uint16_t r{0}, g{0}, b{0};
    float h{0.0f}, s{0.0f}, c{0.0f};
};

#endif