#include <Arduino.h>

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
}

void TaskMove::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    bool left_pressed  = (digitalRead(robot.touch_pin_l) == LOW);
    bool right_pressed = (digitalRead(robot.touch_pin_r) == LOW);

    if (robot.color.get_current_color() == COLOR_BLACK) {
        robot.rpm = 0;
        robot.steer = 0;
        robot.tasks_move.push(TaskBlack());
        done();
        return;
    }

    if (left_pressed != right_pressed) {
        if (!touch_was_pressed) {
            touch_was_pressed = true;
            touch_start_time = elapsed_ms;
        }
        if (elapsed_ms - touch_start_time < 300) {
            robot.rpm = 0;
            robot.steer = 0;
            return;
        }
        robot.rpm = 0;
        robot.steer = 0;

        float dist_u = robot.dist_up.get();
        if (dist_u < 30 && dist_u != 0) {
            robot.rpm = 0;   // Сбрасываем скорость перед выходом
            robot.steer = 0;
            done();          // Завершаем текущую задачу
            return;          // Выходим из on_execute, не добавляя новые задачи!
        }

        robot.tasks_move.push(TaskMove(
            SpeedProfile(30, 100, Quad_MM(300), Quad_MM(200), Quad_MM(50)),
            PID(0.15, 0, 0.04, -200, 200),
            PID(1.1, 0, 0.1, -30, 30)
        ));
        if (left_pressed)
            robot.tasks_move.push(TaskHit(TaskHit::LEFT));
        else
            robot.tasks_move.push(TaskHit(TaskHit::RIGHT));
        done();
        return;
    }
    touch_was_pressed = false;

    float relative_yaw = robot.imu.ypr[0] - yaw_now;
    if (relative_yaw > 180.0f) relative_yaw -= 360.0f;
    else if (relative_yaw < -180.0f) relative_yaw += 360.0f;

    float sensor_ratio = 0.6f, yaw_correction = cosf(abs(constrain(relative_yaw, -90, 90)) * DEG_TO_RAD), max_dist = 120.0f;

    float d_left = robot.dist_left.get() * yaw_correction;
    float d_right = robot.dist_right.get() * yaw_correction;
    float a_left = robot.dist_ang_left.get() * yaw_correction;
    float a_right = robot.dist_ang_right.get() * yaw_correction;

    bool d_left_ok = (d_left >= 0.1f) && (d_left <= max_dist);
    bool a_left_ok = (a_left >= 0.1f) && (a_left <= max_dist / sensor_ratio);
    bool d_right_ok = (d_right >= 0.1f) && (d_right <= max_dist);
    bool a_right_ok = (a_right >= 0.1f) && (a_right <= max_dist / sensor_ratio);

    //LOG_INFO(d_left, d_right, a_left, a_right);

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
            float slip_factor_down = 1.0f - (sin_pitch * 0.7f);
            slip_compensation = 1.0f / slip_factor_down;
        }
    }

    float proj_total = proj_yaw * proj_pitch * slip_compensation;
    if (proj_total < 0.05f) proj_total = 0.05f;
    if (proj_total > 2.5f) proj_total = 2.5f;

    progress_encoder += delta_encoder * proj_total;
    last_encoder = encoder_now;

    if (progress_encoder < speed_profile.get_len()) {
        float speed = speed_profile.compute(progress_encoder);

        if (absolute_pitch > 4.0f) {
            if (pitch_val > 4.0f) {
                speed = speed * 0.6f;
            } else {
                speed = speed * 0.6f;
            }
            if (speed < 20.0f) {
                speed = 20.0f;
            }
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


        bool right_ok = d_right_ok || a_right_ok;
        float right = 0.0f;
        if (right_ok) {
            right = (d_right_ok && a_right_ok) ? (d_right + a_right) / 2.0f 
                : (d_right_ok) ? d_right : a_right;
        }

        bool left_ok = d_left_ok || a_left_ok;
        float left = 0.0f;
        if (left_ok) {
            left = (d_left_ok && a_left_ok) ? (d_left + a_left) / 2.0f 
                : (d_left_ok) ? d_left : a_left;
        }

        float select_angle = 0.0f;
        if (left_ok && right_ok) {
            select_angle = (right - left) * 0.1f;
        } else if (left_ok || right_ok) {
            float dist = left_ok ? left : right;
            select_angle = 66.0f - dist;
        }

        //Srobot.steer = pid_yaw.compute(select_angle, relative_yaw) - dist_correction;
    }
    else {
        robot.rpm = 0;
        robot.steer = 0;
        done();
    }
}
