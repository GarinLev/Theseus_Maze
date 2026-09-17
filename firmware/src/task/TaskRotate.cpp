#include "Task.h"
#include <math.h>
#include "Robot.h"

void TaskRotate::on_init() {
    auto& robot = Robot::instance();
    prev_yaw = robot.imu.ypr[0];
    unwrapped = 0.0f;
    robot.rpm = 0;
    robot.steer = 0.0f;
    
    pid.reset(); 
}

void TaskRotate::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();
    
    float delta = robot.imu.ypr[0] - prev_yaw;
    if (delta > 180.0f)  delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;

    unwrapped += delta;
    prev_yaw = robot.imu.ypr[0];

    float error = target_angle - fabsf(unwrapped);
    const float ANGLE_TOLERANCE = 1.0f;

    if (error > ANGLE_TOLERANCE) {
        float speed = pid.compute(0.0f, -error); 
        robot.steer = speed * direction;
    } else {
        robot.steer = 0.0f;
        done();
    }
}
