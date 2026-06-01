#include "Robot.h"
#include "Task.h"
#include "Log.h"

void TaskRotate::execute() {
    if (robot == nullptr) return;

    if (!started) {
        prev_yaw = robot->ypr[0];
        unwrapped = 0.0f;
        started = true;
    }

    float delta = robot->ypr[0] - prev_yaw;
    if (delta > 180.0f) delta -= 360.0f;
    else if (delta < -180.0f) delta += 360.0f;

    unwrapped += delta;
    prev_yaw = robot->ypr[0];

    float progress = fabsf(unwrapped);
    float target = speed_profile.get_len();

    if (progress < target) {
        state = StateTask::RUNNING;
        float speed = speed_profile.compute(progress);
        robot->rpm = speed * direction;
    } else {
        state = StateTask::CLOSE;
        robot->rpm = 0;
    }
}