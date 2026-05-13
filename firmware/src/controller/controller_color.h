#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "../../lib/TCS34725/Adafruit_TCS34725.h"
#include "../../lib/pt/pt.h"

#define COLOR_THRESHOLD 15

class ColorController {
public:
    void init();
    int  update();

    void calibrateWhite();
    void calibrateColor();

    bool isBlue();
    bool isGrey();
    bool isBlack();

private:
    void rgb_to_hsv(uint16_t r, uint16_t g, uint16_t b, uint16_t c, float& h, float& s, float& v);

    Adafruit_TCS34725 tcs;
    pt pt_task;
    unsigned long last_update_ms;
    uint16_t r, g, b, c;
    float h, s, v;
};