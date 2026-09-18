#include <Arduino.h>
#include "Log.h"
#include "Robot.h"
#include "Task.h"

void TaskHit::on_init() {
    auto& robot = Robot::instance();
    start_encoder = robot.quad.encoder();
}


void TaskHit::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    robot.rpm = -90;

    if (elapsed_ms > 2000) {
        robot.rpm = 0;
        robot.steer = 0;
        done();
        return;
    }

    if (mode == RIGHT)
        robot.steer = 30;
    else if (mode == LEFT)
        robot.steer = -30;

    float progress = fabsf(robot.quad.encoder() - start_encoder);
    if (progress >= Quad_MM(75.0f)) {
        robot.rpm = 0;
        robot.steer = 0;
        done();
    }
}
