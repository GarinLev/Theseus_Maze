#pragma once

#include "../node/node_angel.h"
#include "../soft/soft_move.h"

enum Rotates
{
    ROTATE_CW_90,
    ROTATE_CCW_90,
    ROTATE_CW_180,
    ROTATE_CWW_180,
};

struct RotateController {
    struct pt pt_task;
    AngelNode angel;
    SoftMove profile;

    WheelController* wheelA1;
    WheelController* wheelA2;
    WheelController* wheelB1;
    WheelController* wheelB2;

    float start_yaw = 0;
    uint32_t last_time = 0;
    float min_start_rpm = 50.0f;
    bool is_active = false;

    void init() {
        PT_INIT(&pt_task);
        node_angel_init(angel);
    }

    float normalize_angle(float angle) {
        while (angle > 180) angle -= 360;
        while (angle < -180) angle += 360;
        return angle;
    }

    int update() {
        node_angel_run(angel);

        PT_BEGIN(&pt_task);

        for (;;) {
            PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
            last_time = millis();

            if (!is_active) continue;

            float relative_yaw = normalize_angle(angel.ypr[0] - start_yaw);

            if (fabsf(relative_yaw) >= fabsf(profile.L_total)) {
                stopWheels();
                is_active = false;
                continue;
            }

            float profile_speed = SoftMoveGet(&profile, relative_yaw);

            if (fabsf(profile_speed) < min_start_rpm) {
                float sign = (profile.L_total >= 0) ? 1.0f : -1.0f;
                profile_speed = min_start_rpm * sign;
            }

            applySpeed(profile_speed);
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

    void setMove(float start_pos, float total_angel, float acc_angel, float target_v) {
        profile.x_start = start_pos;
        profile.L_total = total_angel;
        profile.d_acc = acc_angel;
        profile.y0 = min_start_rpm;
        profile.y1 = target_v;
    }

    void run(float total_angel, float target_v) {
        PT_INIT(&pt_task);
        last_time = millis();

        start_yaw = angel.ypr[0];

        setMove(0, total_angel, fabsf(total_angel) * 0.45f, target_v);
        is_active = true;
    }
};