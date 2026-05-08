#include "controller_rotate.h"
#include "controller_wheel.h" // ТЕПЕРЬ МОЖНО ПОЛЬЗОВАТЬСЯ ПОЛЯМИ КОЛЕС
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

void RotateController::run(float total_angel, float target_v) {
    PT_INIT(&pt_task);
    start_yaw = angel.ypr[0];
    target_total_angle = total_angel;
    profile.x_start = 0;
    profile.L_total = fabsf(total_angel);
    profile.d_acc = fabsf(total_angel) * 0.4f;
    profile.y0 = min_start_rpm;
    profile.y1 = target_v;
    is_active = true;
}