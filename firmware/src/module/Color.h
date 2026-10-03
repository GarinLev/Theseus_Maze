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
    void dump_calibration_code();

    float distance_to(const ColorVector& target) const;
    ColorType get_current_color(float max_distance = 0.50f) const;

    float read_normalized();
    void handle_command(const String& cmd);

    // --- Методы для накопления и получения среднего значения ---
    void push_median();
    ColorVector get_median() const;
    ColorType get_median_color(float max_distance = 0.50f) const;
    void reset_median();
    void clear_median() { reset_median(); }

private:
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

    const float Rf = 0.85f;
    const float Gf = 0.91f;
    const float Bf = 1.40f;

    uint8_t pin_led = A1;
    uint16_t raw_led = 0;
    float led_max = 670.0f;
    float led_min = 550.0f;

    float c_min = 12.0f;
    float c_max = 350.0f;

    void hsv();

    uint16_t r{0}, g{0}, b{0}, c{0};
    float h{0.0f}, s{0.0f}, a{0.0f}, c_norm{0.0f};

    // Накопители для вычисления среднего
    float median_sum_cos{0.0f};
    float median_sum_sin{0.0f};
    float median_sum_s{0.0f};
    float median_sum_a{0.0f};
    float median_sum_c{0.0f};
    uint32_t median_count{0};
    
ColorVector targets[5] = {
    { 296.4f, 0.03f, 1.00f, 0.70f }, // WHITE
    { 210.9f, 0.64f, 0.21f, 0.21f }, // BLUE
    { 349.4f, 0.46f, 0.02f, 0.01f }, // BLACK
    { 330.0f, 0.14f, 0.54f, 0.36f }, // SILVER
    { 352.1f, 0.75f, 0.37f, 0.13f }  // RED
};
};

#endif // FIRMWARE_COLOR_H