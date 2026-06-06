#include <Arduino.h>

#include "Log.h"
#include "Robot.h"
#include "Task.h"

void TaskBlack::on_init() {
    auto& robot = Robot::instance();
    robot.is_last_black = true;
    back_start_encoder = robot.quad.encoder();
}

void TaskBlack::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    switch (step) {
        case Step::BACK: {
            robot.rpm = -30;
            robot.steer = 0;

            float progress_back = fabsf(robot.quad.encoder() - back_start_encoder);
            float target_back_ticks = Quad_MM(150.0f);

            if (progress_back >= target_back_ticks) {
                robot.rpm = 0;
                robot.steer = 0;
                step = Step::STOP;
            }
            break;
        }

        case Step::STOP: {
            robot.rpm = 0;
            robot.steer = 0;
            done();
            break;
        }
    }
}
