#pragma once
#include <Arduino.h>
#include <Servo.h>
#include "../../lib/pt/pt.h"
#include "../soft/soft_set.h"

struct ServoNode {
    struct pt pt;
    Servo motor;
    SoftSet movement;
        
    uint8_t pin;
    float current_angle;

    uint32_t last_update;
    uint32_t end_time;
    bool has_new_task;
};

void node_servo_init(ServoNode& ctx, uint8_t pin, float init_angle);
int node_servo_run(ServoNode& ctx);