#pragma once
#include "../../lib/pt/pt.h"
#include "../node/node_angel.h"
#include "../soft/soft_move.h"

struct WheelController;

struct RotateController {
    struct pt pt_task;
    AngelNode angel;
    SoftMove profile;

    WheelController* wheelA1, * wheelA2, * wheelB1, * wheelB2;
    

    float start_yaw = 0;
    float target_total_angle = 0;
    uint32_t last_time = 0;
    float min_start_rpm = 45.0f;
    bool is_active = false;

    void init();
    float normalize_angle(float angle);
    int update();
    void applySpeed(float speed);
    void stopWheels();
    void run(float total_angle, float target_v);
};