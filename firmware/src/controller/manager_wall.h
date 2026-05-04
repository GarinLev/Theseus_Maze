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
        node_pi_init(centerNode);
    }

    int update() {
        node_pi_run(centerNode);

        PT_BEGIN(&pt_task);
        for (;;) {
            PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
            last_time = millis();


            int stopped_count = 0;
            if (wheelA1->state == WheelController::STOP) stopped_count++;
            if (wheelA2->state == WheelController::STOP) stopped_count++;
            if (wheelB1->state == WheelController::STOP) stopped_count++;
            if (wheelB2->state == WheelController::STOP) stopped_count++;

            if (stopped_count >= 2 && (wheelA1->is_moving)) {
                wheelA1->stop();
                wheelA2->stop();
                wheelB1->stop();
                wheelB2->stop();
            }
            if (wheelA1->is_moving) {
                bool left_ok = sens_left->correction_valid;
                bool right_ok = sens_right->correction_valid;

                if (left_ok && right_ok) {
                    centerTopic.value = (float)sens_left->nodeUp.dist - (float)sens_right->nodeUp.dist;
                    float angle_out = centerNode.value_out;
                    


                    sens_left->wallNode.setpoint = angle_out;
                    sens_right->wallNode.setpoint = -angle_out;

                    Serial.print("center-ok"); Serial.print("\t");
                    Serial.print(angle_out); Serial.print("\t");
                }
                else {
                    centerNode.integral = 0;
                    sens_left->wallNode.setpoint = 0;
                    sens_right->wallNode.setpoint = 0;
                }

                int16_t target_correction = 0;

                if (left_ok && right_ok) {
                    target_correction = -sens_right->correction;

                    Serial.print("rl-ok"); Serial.print("\t");
                    Serial.print(sens_right->correction); Serial.print("\t");
                    Serial.print(sens_right->correction);
                }
                else if (right_ok) {
                    Serial.print("r-ok"); Serial.print("\t");
                    Serial.print(sens_right->correction); Serial.print("\t");

                    target_correction = -sens_right->correction;
                }
                else if (left_ok) {
                    Serial.print("l-ok"); Serial.print("\t");
                    Serial.print(sens_left->correction); Serial.print("\t");

                    target_correction = sens_left->correction;
                }

                Serial.print('\n');



                wheelA1->speed_offset = -target_correction;
                wheelA2->speed_offset = -target_correction;
                wheelB1->speed_offset = target_correction;
                wheelB2->speed_offset = target_correction;
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