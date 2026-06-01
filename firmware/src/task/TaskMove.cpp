#include <Arduino.h>

#include "Log.h"
#include "Robot.h"
#include "Task.h"


void TaskMove::execute() {
    if (robot == nullptr) return;

    if (!started) {
        start_encoder = robot->quad.encoder();
        last_encoder = start_encoder;
        progress_encoder = 0.0f;
        yaw_now = robot->ypr[0];
        started = true;
        pid_yaw.reset();
        pid_dist.reset();
    }

    float relative_yaw = robot->ypr[0] - yaw_now;
    if (relative_yaw > 180.0f) relative_yaw -= 360.0f;
    else if (relative_yaw < -180.0f) relative_yaw += 360.0f;

    float pitch_val = robot->ypr[1];
    float absolute_pitch = fabsf(pitch_val);

    const float encoder_now = robot->quad.encoder();
    float delta_encoder = encoder_now - last_encoder;

    float proj_yaw = cosf(relative_yaw * DEG_TO_RAD);
    float proj_pitch = cosf(absolute_pitch * DEG_TO_RAD);

    float slip_compensation = 1.0f;

    if (absolute_pitch > 4.0f) {
        float sin_pitch = sinf(absolute_pitch * DEG_TO_RAD);

        if (pitch_val < 0.0f) {
            float slip_factor = 1.0f + (sin_pitch * 0.8f);
            slip_compensation = 1.0f / slip_factor;
        } else {
            slip_compensation = 1.0f + (sin_pitch * 0.9f);
        }
    }

    float proj_total = proj_yaw * proj_pitch * slip_compensation;

    if (proj_total < 0.05f) proj_total = 0.05f;
    if (proj_total > 2.5f) proj_total = 2.5f;

    progress_encoder += delta_encoder * proj_total;
    last_encoder = encoder_now;

    LOG_TRACE("dEnc:", delta_encoder,
              " Pitch:", pitch_val,
              " pYaw:", proj_yaw,
              " pPitch:", proj_pitch,
              " Slip:", slip_compensation,
              " Total:", proj_total,
              " Prog:", progress_encoder,
              " Target:", speed_profile.get_len());

    if (progress_encoder < speed_profile.get_len()) {
        state = StateTask::RUNNING;

        float speed = speed_profile.compute(progress_encoder);

        if (pitch_val > 4.0f && speed < 30.0f && speed > 5.0f) {
            speed = 30.0f;
        }

        robot->rpm = speed;

        float value_right = robot->dist_right.get();
        float value_left = robot->dist_left.get();

        bool correct = value_left > 10 && value_left <= 200 &&
                       value_right > 10 && value_right <= 200;

        float dist_correction = 0;

        if (correct) {
            float dist_err = 0;
            dist_err = (value_right - value_left) * proj_yaw;
            dist_correction = pid_dist.compute(0, dist_err);
        } else {
            pid_dist.reset();
        }

        robot->steer = pid_yaw.compute(0, relative_yaw) - dist_correction;

    } else {
        state = StateTask::CLOSE;
        robot->rpm = 0;
        robot->steer = 0;
    }
}
