#ifndef FIRMWARE_COLOR_H
#define FIRMWARE_COLOR_H

#include <Arduino.h>
#include <Adafruit_TCS34725.h>

struct ColorVector {
    float h;
    float s;
    float a;      // 0.0 .. 1.0 (Отражение на Pin A1)
    float c_norm; // 0.0 .. 1.0 (Clear-канал TCS34725)
};

enum ColorType : uint8_t {
    COLOR_WHITE = 0,
    COLOR_BLUE = 1,
    COLOR_BLACK = 2,
    COLOR_SILVER = 3,
    COLOR_RED = 4,
    COLOR_UNKNOWN = 255
};

class Color {
public:
    void init();
    void update();
    void log();

    void calibrate(ColorType color);

    float distance_to(const ColorVector& target) const;
    ColorType get_current_color(float max_distance = 0.45f) const;

    float read_normalized();
    void handle_command(const String& cmd);

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

    const float Rf = 0.85f;
    const float Gf = 0.91f;
    const float Bf = 1.40f;

    uint8_t pin_led = A1;
    uint16_t raw_led = 0;
    float led_max = 670.0f;
    float led_min = 565.0f;

    float c_min = 12.0f;
    float c_max = 210.0f;

    void hsv();

    uint16_t r{0}, g{0}, b{0}, c{0};
    float h{0.0f}, s{0.0f}, a{0.0f}, c_norm{0.0f};

    ColorVector targets[5] = {
        {   0.0f, 0.02f, 1.00f, 1.00f }, // WHITE
        { 224.0f, 0.58f, 0.08f, 0.15f }, // BLUE
        { 345.0f, 0.39f, 0.00f, 0.00f }, // BLACK
        {   0.0f, 0.05f, 0.55f, 0.65f }, // SILVER
        {  10.0f, 0.65f, 0.15f, 0.25f }  // RED
    };
};

#endif                                                      