#include <Arduino.h>

#include "Log.h"
#include "Robot.h"
#include "Task.h"


void TaskMove::on_init() {
    auto& robot = Robot::instance();
    start_encoder = robot.quad.encoder();
    last_encoder = start_encoder;
    progress_encoder = 0.0f;
    yaw_now = robot.imu.ypr[0];
    pid_yaw.reset();
    pid_dist.reset();
    step = Step::DRIVE;
}

void TaskMove::on_execute() {
    auto& robot = Robot::instance();

    switch (step) {
        case Step::DRIVE: {
            if (robot.color.get_current_color() == COLOR_BLACK) {
                LOG_INFO("Black color detected! Switching to Step::BACK.");

                robot.rpm = 0;
                robot.steer = 0;

                back_start_encoder = robot.quad.encoder();
                step = Step::BACK;
                break;
            }

            float relative_yaw = robot.imu.ypr[0] - yaw_now;
            if (relative_yaw > 180.0f) relative_yaw -= 360.0f;
            else if (relative_yaw < -180.0f) relative_yaw += 360.0f;

            float pitch_val = robot.imu.ypr[1];
            float absolute_pitch = fabsf(pitch_val);

            const float encoder_now = robot.quad.encoder();
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

            if (progress_encoder < speed_profile.get_len()) {
                float speed = speed_profile.compute(progress_encoder);
                if (pitch_val > 4.0f && speed < 30.0f && speed > 5.0f) {
                    speed = 30.0f;
                }

                robot.rpm = speed;

                float value_right = robot.dist_right.get();
                float value_left = robot.dist_left.get();

                bool correct = value_left > 10 && value_left <= 200 &&
                               value_right > 10 && value_right <= 200;

                float dist_correction = 0;
                if (correct) {
                    float dist_err = (value_right - value_left) * proj_yaw;
                    dist_correction = pid_dist.compute(0, dist_err);
                } else {
                    pid_dist.reset();
                }

                robot.steer = pid_yaw.compute(0, relative_yaw) - dist_correction;
            } else {
                robot.rpm = 0;
                robot.steer = 0;
                done();
            }
            break;
        }

        case Step::BACK: {
            robot.rpm = -30;
            robot.steer = 0;

            float progress_back = fabsf(robot.quad.encoder() - back_start_encoder);
            float target_back_ticks = Quad_MM(150.0f);

            if (progress_back >= target_back_ticks) {
                LOG_INFO("Backed up 15 cm successfully. Stopping.");
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