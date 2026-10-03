#include <Arduino.h>

#include "Robot.h"
#include "Task.h"

void TaskMove::on_init() {
    auto& robot = Robot::instance();
    start_encoder = robot.quad.encoder();
    last_encoder = start_encoder;
    progress_encoder = 0.0f;

    robot.color.reset_median();

    // Захватываем ближайшую идеальную ось лабиринта (0, 90, 180, -90)
    yaw_now = robot.imu.get_nearest_90();
    if (yaw_now > 180.0f) {
        yaw_now -= 360.0f; // 270° -> -90°
    }

    pid_yaw.reset();
    pid_dist.reset();
    pid_one.reset();
    reg_mode = 0;
    reg_cand = 0;
    reg_switch_ms = 0;
    reg_initialized = false;

    // Сброс состояния рампы
    ramp_mode = false;
    ramp_leveled = false;
    ramp_flat_start_enc = 0.0f;
    ramp_detect_ms = 0;
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

    if ((left_pressed != right_pressed) && fabsf(progress_encoder) >= fabsf(speed_profile.get_len()) * 0.6f) {
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
            done();
            return;
        }

        robot.link.push_forward_sequence(robot);
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

    float sensor_ratio_left = 0.77f, sensor_ratio_right = 0.529f;
    float yaw_correction = cosf(abs(constrain(relative_yaw, -90, 90)) * DEG_TO_RAD);
    float max_dist = 160.0f;

    float d_left_raw = robot.dist_left.get();
    float d_right_raw = robot.dist_right.get();
    float a_left_raw = robot.dist_ang_left.get() * sensor_ratio_left;
    float a_right_raw = robot.dist_ang_right.get() * sensor_ratio_right;

    float d_left = d_left_raw * yaw_correction;
    float d_right = d_right_raw * yaw_correction;
    float a_left = a_left_raw * yaw_correction;
    float a_right = a_right_raw * yaw_correction;

    bool d_left_ok = (d_left >= 0.1f) && (d_left <= max_dist);
    bool a_left_ok = (a_left >= 0.1f) && (a_left <= max_dist);
    bool d_right_ok = (d_right >= 0.1f) && (d_right <= max_dist);
    bool a_right_ok = (a_right >= 0.1f) && (a_right <= max_dist);

    float pitch_val = robot.imu.ypr[1];
    float absolute_pitch = fabsf(pitch_val);

    const float encoder_now = robot.quad.encoder();
    float delta_encoder = encoder_now - last_encoder;

    float proj_yaw = cosf(relative_yaw * DEG_TO_RAD);
    float proj_pitch = cosf(absolute_pitch * DEG_TO_RAD);

    float slip_compensation = 1.0f;
    if (absolute_pitch > 15.0f) {
        slip_compensation = 0;
    }   
    LOG_INFO("absolute_pitch", absolute_pitch);   

    float proj_total = proj_yaw * proj_pitch * slip_compensation;

    progress_encoder += delta_encoder * proj_total;
    
    // Защита от люфта: если едем вперед, прогресс не должен быть отрицательным
    if (speed_profile.get_len() >= 0.0f && progress_encoder < 0.0f) {
        progress_encoder = 0.0f;
    }
    
    last_encoder = encoder_now;

    // --- ПРОВЕРКА УСЛОВИЯ РАМПЫ (С ЗАЩИТОЙ ОТ КОЧЕК/БАМПОВ) ---
    const float half_cell = fabsf(speed_profile.get_len()) * 0.5f;

    if (!ramp_mode) {
        // Условие: больше половины клетки и угол pitch > 17
        if (fabsf(progress_encoder) >= half_cell && absolute_pitch > 17.0f) {
            ramp_detect_ms += dt;
            // Подтверждаем рампу, если угол держится не менее 250 мс
            if (ramp_detect_ms >= 250) {
                ramp_mode = true;
            }
        } else {
            ramp_detect_ms = 0;
        }
    }

    // Обработка логики преодоления и съезда с рампы
    if (ramp_mode) {
        if (!ramp_leveled) {
            // Едем вперед, пока pitch не уменьшится до 5 градусов
            if (absolute_pitch <= 5.0f) {
                ramp_leveled = true;
                ramp_flat_start_enc = encoder_now; // Фиксируем позицию для отсчета 15 см
            }
        } else {
            // Проезжаем ровно 15 см (150 мм) после выравнивания
            if (fabsf(encoder_now - ramp_flat_start_enc) >= Quad_MM(150)) {
                robot.rpm = 0;
                robot.steer = 0;
                done();
                return;
            }
        }
    }

    // Проверяем: продолжаем ехать или останавливаемся
    bool profile_active = (fabsf(progress_encoder) < fabsf(speed_profile.get_len()));

    if (profile_active || ramp_mode) {
        if (fabsf(progress_encoder) >= fabsf(speed_profile.get_len()) * 0.6f) {
            robot.color.push_median();
        }

        // Если градус больше 10 или мы в режиме рампы — отключаем торможение по профилю
        bool no_decel = (absolute_pitch > 10.0f) || ramp_mode;
        float speed = speed_profile.compute(progress_encoder, no_decel);

        robot.rpm = speed;

        // --- РЕГУЛЯТОР УДЕРЖАНИЯ КУРСА И СТЕН ---
        const float a_match_tol = 20.0f;
        bool a_left_use  = a_left_ok  && fabsf(d_left  - a_left)  <= a_match_tol;
        bool a_right_use = a_right_ok && fabsf(d_right - a_right) <= a_match_tol;

        bool right_ok = d_right_ok || a_right_use;
        float right = 0.0f;
        if (right_ok) {
            right = (d_right_ok && a_right_use) ? (d_right + a_right) / 2.0f
                : (d_right_ok) ? d_right : a_right;
        }

        bool left_ok = d_left_ok || a_left_use;
        float left = 0.0f;
        if (left_ok) {
            left = (d_left_ok && a_left_use) ? (d_left + a_left) / 2.0f
                : (d_left_ok) ? d_left : a_left;
        }

        uint8_t cand = 0;
        if (left_ok && right_ok) {
            cand = 0;
        } else if (left_ok || right_ok) {
            cand = 1;
        } else {
            cand = 2;
        }

        if (cand != reg_cand) {
            reg_cand = cand;
            reg_switch_ms = 0;
        } else if (reg_mode != cand) {
            if (!reg_initialized) {
                reg_mode = cand;
                reg_switch_ms = 0;
                pid_dist.reset();
                pid_one.reset();
                pid_yaw.reset();
            } else {
                reg_switch_ms += dt;
                if (reg_switch_ms >= 25) {
                    reg_mode = cand;
                    reg_switch_ms = 0;
                    pid_dist.reset();
                    pid_one.reset();
                    pid_yaw.reset();
                }
            }   
        } else {
            reg_switch_ms = 0;
        }
        reg_initialized = true;

        float wall_err = 0.0f;
        float yaw_target = 0.0f;

        if (reg_mode == 0) {
            wall_err = left - right;
            yaw_target = pid_dist.compute(0, wall_err);
        } else if (reg_mode == 1) {
            wall_err = left_ok ? (left - 100.0f) : (100.0f - right);
            yaw_target = pid_one.compute(0, wall_err);
        } else {
            wall_err = 0.0f;
            yaw_target = 0.0f;
        }

        float yaw_out = pid_yaw.compute(yaw_target, relative_yaw);
        robot.steer = yaw_out;

    } else {
        // Обычная остановка в конце клетки (если рампы не было)
        robot.rpm = 0;
        robot.steer = 0;
        done();
    }
}