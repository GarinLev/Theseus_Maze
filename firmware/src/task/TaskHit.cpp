#include <Arduino.h>
#include "Log.h"
#include "Robot.h"
#include "Task.h"

void TaskHit::on_init() {
    auto& robot = Robot::instance();
    start_encoder = robot.quad.encoder();
}


void TaskHit::on_execute() {
    auto& robot = Robot::instance();

    robot.rpm = -30;

    if (mode == RIGHT)
        robot.steer = 20;
    else if (mode == LEFT)
        robot.steer = -20;

    float progress = fabsf(robot.quad.encoder() - start_encoder);
    if (progress >= Quad_MM(75.0f)) {
        robot.rpm = 0;
        robot.steer = 0;
        done();
    }
}
