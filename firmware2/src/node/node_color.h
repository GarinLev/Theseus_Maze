#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "../../lib/TCS34725/Adafruit_TCS34725.h"
#include "../../lib/pt/pt.h"

struct ColorNode {
    struct pt pt;
    Adafruit_TCS34725 tcs;

    uint16_t r, g, b, c;

    float h;
    float s;
    float v;

    uint32_t last_read;
};

void node_color_init(ColorNode& ctx);
int node_color_run(ColorNode& ctx);