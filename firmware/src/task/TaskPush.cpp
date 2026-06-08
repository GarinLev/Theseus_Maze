#include "Robot.h"
#include "Task.h"

#define ServoController_Right 0
#define ServoController_Left 140
#define ServoController_CloseStart 70
#define ServoController_CloseRight 75
#define ServoController_CloseLeft 65

constexpr uint32_t SERVO_MOVE_MS = 1000;
constexpr uint32_t SERVO_HOLD_MS = 500;

void TaskPush::on_init() {
    step = Step::TO_TARGET;

    switch (mode) {
        case Mode::LEFT:
            target_pos = ServoController_Left;
            home_pos = ServoController_CloseLeft;
            pushes_left = 1;
            break;

        case Mode::LEFT_X2:
            target_pos = ServoController_Left;
            home_pos = ServoController_CloseLeft;
            pushes_left = 2;
            break;

        case Mode::RIGHT:
            target_pos = ServoController_Right;
            home_pos = ServoController_CloseRight;
            pushes_left = 1;
            break;

        case Mode::RIGHT_X2:
            target_pos = ServoController_Right;
            home_pos = ServoController_CloseRight;
            pushes_left = 2;
            break;

        default:
            target_pos = ServoController_CloseStart;
            home_pos = ServoController_CloseStart;
            pushes_left = 0;
            done();
            return;
    }

    current_servo_pos = ServoController_CloseStart;

    start_pos = current_servo_pos;
    wait_start_ms = 0;

    auto& robot = Robot::instance();
    robot.rpm = 0; robot.steer = 0;
}

void TaskPush::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    switch (step) {
        case Step::TO_TARGET: {
            uint32_t elapsed = elapsed_ms - wait_start_ms;

            if (elapsed >= SERVO_MOVE_MS) {
                current_servo_pos = target_pos;
                robot.servo.write(current_servo_pos);

                wait_start_ms = elapsed_ms;
                step = Step::WAIT_TARGET;
            } else {
                float progress = (float)elapsed / SERVO_MOVE_MS;
                // Формула Smoothstep для плавного ускорения и замедления
                float smooth_progress = progress * progress * (3.0f - 2.0f * progress);

                current_servo_pos = start_pos + (target_pos - start_pos) * smooth_progress;
                robot.servo.write(current_servo_pos);
            }
            break;
        }

        case Step::WAIT_TARGET: {
            if (elapsed_ms - wait_start_ms >= SERVO_HOLD_MS) {
                start_pos = current_servo_pos;
                wait_start_ms = elapsed_ms;
                step = Step::TO_HOME;
            }
            break;
        }

        case Step::TO_HOME: {
            uint32_t elapsed = elapsed_ms - wait_start_ms;

            if (elapsed >= SERVO_MOVE_MS) {
                current_servo_pos = home_pos;
                robot.servo.write(current_servo_pos);

                pushes_left--;
                wait_start_ms = elapsed_ms;
                step = Step::WAIT_HOME;
            } else {
                float progress = (float)elapsed / SERVO_MOVE_MS;
                // Применяем ускорение и для обратного пути
                float smooth_progress = progress * progress * (3.0f - 2.0f * progress);

                current_servo_pos = start_pos + (home_pos - start_pos) * smooth_progress;
                robot.servo.write(current_servo_pos);
            }
            break;
        }

        case Step::WAIT_HOME: {
            if (elapsed_ms - wait_start_ms >= SERVO_HOLD_MS) {
                if (pushes_left > 0) {
                    start_pos = current_servo_pos;
                    wait_start_ms = elapsed_ms;
                    step = Step::TO_TARGET;
                } else {
                    done();
                }
            }
            break;
        }
    }
}