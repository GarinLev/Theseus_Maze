#include <math.h>

#include "Task.h"

#include "Log.h"

void TaskMove::execute() {
    if (encoder_now == nullptr) return;
    if (rpm == nullptr) return;
    if (yaw == nullptr) return;

    if (!started) {
        start_encoder = *encoder_now;
        yaw_now = *yaw;
        started = true;
        pid.reset();
    }

    float progress_encoder = *encoder_now - start_encoder;
    if (progress_encoder < speed_profile.get_len()) {
        state = StateTask::RUNNING;
        float speed = speed_profile.compute(progress_encoder);
        *rpm = speed;

        float relative_yaw = *yaw - yaw_now;
        if (relative_yaw > 180.0f) relative_yaw -= 360.0f;
        else if (relative_yaw < -180.0f) relative_yaw += 360.0f;
        *steer = pid.compute(0, relative_yaw);
    } else {
        state = StateTask::CLOSE;
        *rpm = 0;
        *steer = 0;
    }
}

void TaskRotate::execute() {
    if (yaw == nullptr) return;
    if (rpm == nullptr) return;

    if (!started) {
        prev_yaw = *yaw;
        unwrapped = 0.0f;
        started = true;
    }

    float delta = *yaw - prev_yaw;
    if (delta > 180.0f) delta -= 360.0f;
    else if (delta < -180.0f) delta += 360.0f;

    unwrapped += delta;
    prev_yaw = *yaw;

    float progress = fabsf(unwrapped);
    float target = speed_profile.get_len();

    if (progress < target) {
        state = StateTask::RUNNING;
        float speed = speed_profile.compute(progress);
        *rpm = speed * direction;
    } else {
        state = StateTask::CLOSE;
        *rpm = 0;
    }
}

