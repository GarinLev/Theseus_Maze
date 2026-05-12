#pragma once
#include "controller_wall.h"
#include "controller_wheel.h"
#include "../../lib/pt/pt.h"

struct WallManager {
    WallController* sens_right;
    WallController* sens_left;

    WheelController* wheelA1;
    WheelController* wheelA2;
    WheelController* wheelB1;
    WheelController* wheelB2;

    PI_Node centerNode;
    PI_Topic centerTopic;

    struct pt pt_task;
    uint32_t last_time = 0;

    void init() {
        PT_INIT(&pt_task);

        centerNode.topic = &centerTopic;
        centerNode.Kp = 0.05f;
        centerNode.Ki = 0.0f;
        centerNode.dt = 25;
        centerNode.setpoint = 0;
        centerNode.integral = 0;
    }

    int update() {
        PT_BEGIN(&pt_task);
        for (;;) {
            PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
            last_time = millis();

            if (wheelA1->is_moving) {
                bool left_ok = sens_left->correction_valid;
                bool right_ok = sens_right->correction_valid;

                if (left_ok && right_ok) {
                    centerTopic.value = (float)sens_left->nodeUp.dist - (float)sens_right->nodeUp.dist;

                    float err = centerNode.setpoint - centerTopic.value;
                    centerNode.integral += err * (centerNode.dt / 1000.0f);
                    centerNode.integral = constrain(centerNode.integral, -150.0f, 150.0f);
                    centerNode.value_out = (err * centerNode.Kp) + (centerNode.integral * centerNode.Ki);

                    float angle_out = centerNode.value_out;
                    sens_left->wallNode.setpoint = angle_out;
                    sens_right->wallNode.setpoint = -angle_out;
                }
                else {
                    centerNode.integral = 0;
                    sens_left->wallNode.setpoint = 0;
                    sens_right->wallNode.setpoint = 0;
                }

                int16_t target_correction = 0;
                if (right_ok && left_ok) {
                    target_correction = (sens_right->correction - sens_left->correction) / 2;
                }
                else if (right_ok) {
                    target_correction = sens_right->correction;
                }
                else if (left_ok) {
                    target_correction = -sens_left->correction;
                }

                wheelA1->speed_offset = target_correction;
                wheelA2->speed_offset = target_correction;
                wheelB1->speed_offset = -target_correction;
                wheelB2->speed_offset = -target_correction;

            }
            else {
                wheelA1->speed_offset = 0;
                wheelA2->speed_offset = 0;
                wheelB1->speed_offset = 0;
                wheelB2->speed_offset = 0;
                centerNode.integral = 0;
            }
        }
        PT_END(&pt_task);
    }
};