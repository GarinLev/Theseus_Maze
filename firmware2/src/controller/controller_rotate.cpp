#include "controller_rotate.h"
#include "controller_wheel.h"
#include <Arduino.h>

void RotateController::init() {
    PT_INIT(&pt_task);
    node_angel_init(angel);
}

float RotateController::normalize_angle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

int RotateController::update() {
    node_angel_run(angel);
    PT_BEGIN(&pt_task);
    for (;;) {
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
        last_time = millis();
        if (!is_active) continue;

        float current_yaw = angel.ypr[0];
        float error = normalize_angle(target_total_angle - normalize_angle(current_yaw - start_yaw));

        if (fabsf(error) < 1.2f) {
            stopWheels();
            is_active = false;
            continue;
        }

        float traveled = fabsf(target_total_angle) - fabsf(error);
        float profile_speed = max(SoftMoveGet(&profile, max(0.0f, traveled)), min_start_rpm);
        applySpeed(profile_speed * ((error > 0) ? 1.0f : -1.0f));
    }
    PT_END(&pt_task);
}

void RotateController::applySpeed(float speed) {
    if (!wheelA1) return;
    wheelA1->speed_offset = 0; wheelA2->speed_offset = 0;
    wheelB1->speed_offset = 0; wheelB2->speed_offset = 0;
    wheelA1->setSpeed(speed); wheelA2->setSpeed(speed);
    wheelB1->setSpeed(-speed); wheelB2->setSpeed(-speed);
}

void RotateController::stopWheels() {
    if (!wheelA1) return;
    wheelA1->stop(); wheelA2->stop();
    wheelB1->stop(); wheelB2->stop();
}


void RotateController::run(Rotates rotate_need, float target_v) {
    PT_INIT(&pt_task);
    
    static const int16_t r90 = 85;
    static const int16_t r180 = 175;
    static const int16_t rotation_matrix[4][4] = {
        {    0,  -r90, r180,   r90 },
        {   r90,    0, -r90,  r180 },
        {  r180,  r90,    0,  -r90 },
        {  -r90, r180,  r90,     0 }
    };

    int16_t total_angle = rotation_matrix[rotate_now][rotate_need];
    rotate_now = rotate_need;

    if (total_angle == 0) {
        is_active = false;
        return;
    }

    start_yaw = angel.ypr[0];
    target_total_angle = total_angle;

    float abs_angle = fabsf((float)total_angle);

    profile.x_start = 0;
    profile.L_total = abs_angle;
    profile.d_acc = abs_angle * 0.4f;
    profile.y0 = min_start_rpm;
    profile.y1 = target_v;

    is_active = true;
}