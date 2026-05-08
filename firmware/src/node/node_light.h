#pragma once

#include "../../lib/pt/pt.h"
#include "../macro.h"
#include "../../lib/TCS34725/Adafruit_TCS34725.h"

struct LightNode {
    struct pt pt;

    Adafruit_TCS34725 light;
    uint16_t r1, g1, b1, c1;

};

void node_light_init(AngelNode&ctx);
int  node_light_run(AngelNode&ctx);