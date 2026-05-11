#pragma once
#include <Arduino.h>
#include "../../lib/pt/pt.h"

#define COLOR_DEBTH 3
#include "../../lib/microLED/microLED.h"
    
#define LightController_Time 500

struct LightController {
    struct pt pt_task;
    uint16_t timer;

    enum Colors {
        RED,
        GREEN,
        BLUE
    };
    Colors color;

    microLED<11, 43, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER, SAVE_MILLIS> strip;
    bool isEnable = false;

    void init();
    void on();
    void off();
    int update();
};
