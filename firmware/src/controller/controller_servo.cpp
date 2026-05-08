#pragma once
#include "../../lib/pt/pt.h"
#include "../soft/soft_set.h"
#include <Servo.h>

#define ServoController_Right 0
#define ServoController_Left 140
#define ServoController_CloseStart 70

struct ServoController {
    struct pt pt_task;
    Servo servo;
    uint16_t pin;
    SoftSet soft;
    uint32_t timer = 0;
    uint32_t start_time;
    float position = ServoController_CloseStart;
    bool is_active = false;

    void init();
    void set(float _pos);
    int update();
};