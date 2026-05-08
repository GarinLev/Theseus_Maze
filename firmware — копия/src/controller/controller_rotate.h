#pragma once

#include "../node/node_angel.h"
#include "../soft/soft_move.h"

struct RotateController {
    struct pt pt_task;
    AngelNode angel;
    SoftMove profile;

    WheelController* wheelA1;
    WheelController* wheelA2;
    WheelController* wheelB1;
    WheelController* wheelB2;

    float start_yaw = 0;
    float target_total_angle = 0;
    uint32_t last_time = 0;
    float min_start_rpm = 45.0f;
    bool is_active = false;

    void init() {
        PT_INIT(&pt_task);
        node_angel_init(angel);
    }

    float normalize_angle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    int update() {
        node_angel_run(angel);

        PT_BEGIN(&pt_task);

        for (;;) {
            PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
            last_time = millis();

            if (!is_active) continue;

            float current_yaw = angel.ypr[0];
            float relative_yaw = normalize_angle(current_yaw - start_yaw);

            float error = normalize_angle(target_total_angle - relative_yaw);

           
            if (fabsf(error) < 1.2f) {
                stopWheels();
                is_active = false;
                continue;
            }

            float traveled = fabsf(target_total_angle) - fabsf(error);
            if (traveled < 0) traveled = 0;

            float profile_speed = SoftMoveGet(&profile, traveled);

            if (profile_speed < min_start_rpm) {
                profile_speed = min_start_rpm;
            }

            float sign = (error > 0) ? 1.0f : -1.0f;
            applySpeed(profile_speed * sign);
        }

        PT_END(&pt_task);
    }

    void applySpeed(float speed) {
        wheelA1->speed_offset = 0; wheelA2->speed_offset = 0;
        wheelB1->speed_offset = 0; wheelB2->speed_offset = 0;

        wheelA1->setSpeed(speed);
        wheelA2->setSpeed(speed);
        wheelB1->setSpeed(-speed);
        wheelB2->setSpeed(-speed);
    }

    void stopWheels() {
        wheelA1->stop(); wheelA2->stop();
        wheelB1->stop(); wheelB2->stop();
    }

    void run(float total_angel, float target_v) {
        PT_INIT(&pt_task);
        last_time = millis();
        start_yaw = angel.ypr[0];
        target_total_angle = total_angel;

        
        profile.x_start = 0;
        profile.L_total = fabsf(total_angel);
        profile.d_acc = fabsf(total_angel) * 0.40f;
        profile.y0 = min_start_rpm;
        profile.y1 = target_v;

        is_active = true;
    }

    bool isMoving() {
        return is_active;
    }
};
