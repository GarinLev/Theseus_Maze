#include <Arduino.h>

#include "Log.h"
#include "Robot.h"
#include "Task.h"

void TaskMove::execute() {
    if (robot == nullptr) return;

    if (!started) {
        start_encoder = robot->quad.encoder();
        yaw_now = robot->ypr[0];
        started = true;
        pid_yaw.reset();
        pid_dist.reset();
    }

    float relative_yaw = robot->ypr[0] - yaw_now;
    if (relative_yaw > 180.0f) relative_yaw -= 360.0f;
    else if (relative_yaw < -180.0f) relative_yaw += 360.0f;

    const float encoder_now = robot->quad.encoder();

    const float progress_encoder = encoder_now - start_encoder;
    if (progress_encoder < speed_profile.get_len()) {
        state = StateTask::RUNNING;
        float speed = speed_profile.compute(progress_encoder);
        robot->rpm = speed;

        float proj = cosf(relative_yaw * DEG_TO_RAD);

        float value_right = robot->dist_right.get();
        float value_left = robot->dist_left.get();
        bool correct = value_left <= 200 && value_right <= 200;

        float dist_correction = 0;
        float dist_err = 0;

        if (correct) {
            dist_err = (value_right - value_left) * proj;
            dist_correction = pid_dist.compute(0, dist_err);
        } else {
            pid_dist.reset();
        }

        LOG_TRACE(robot->dist_left.get(), robot->dist_right.get(), dist_err, dist_correction, proj);
        robot->steer = pid_yaw.compute(0, relative_yaw) - dist_correction;

    } else {
        state = StateTask::CLOSE;
        robot->rpm = 0;
        robot->steer = 0;
    }
}
