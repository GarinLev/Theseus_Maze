#include "Robot.h"
#include "Task.h"

void TaskRotate::on_init() {
    auto& robot = Robot::instance();
    prev_yaw = robot.imu.ypr[0];
    unwrapped = 0.0f;
    robot.rpm = 0;
}

void TaskRotate::on_execute() {
    auto& robot = Robot::instance();
    float delta = robot.imu.ypr[0] - prev_yaw;
    if (delta > 180.0f) delta -= 360.0f;
    else if (delta < -180.0f) delta += 360.0f;

    unwrapped += delta;
    prev_yaw = robot.imu.ypr[0];

    float progress = fabsf(unwrapped);
    float target = speed_profile.get_len();

    if (progress < target) {
        float speed = speed_profile.compute(progress);
        robot.steer = speed * direction;
    } else {
        done();
        robot.steer = 0;
    }
}
