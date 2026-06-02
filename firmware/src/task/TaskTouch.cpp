#include <Arduino.h>
#include "Robot.h"
#include "Task.h"

void TaskTouch::on_init() {
    if (robot == nullptr) return;
    step_timer = millis();
    step = Step::INIT;
}

void TaskTouch::on_execute() {
    float dist = robot->dist_up.get();

    switch (step) {
        case Step::INIT:
            robot->rpm = 0;
            robot->steer = 0;
            if (millis() - step_timer >= 150) {

                if (dist > 200.0f || dist == 0.0f) {
                    done();
                } else {
                    step = Step::SEARCH;
                }
            }
            break;

        case Step::SEARCH:
            robot->rpm = 25;
            robot->steer = 0;
            if (Robot::touch_is()) {
                step_timer = millis();
                step = Step::ALIGN;
            }
            break;

        case Step::ALIGN:
            robot->rpm = 20;
            robot->steer = 0;
            if (millis() - step_timer >= 300) {
                start_encoder = robot->quad.encoder();
                step = Step::BACK;
            }
            break;

        case Step::BACK:
            robot->rpm = -30;
            robot->steer = 0;

            float progress = fabsf(robot->quad.encoder() - start_encoder);

            if (progress >= back_ticks) {
                robot->rpm = 0;
                done();
            }
            break;
    }
}
