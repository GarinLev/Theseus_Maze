#include "Link.h"
#include "Robot.h"
#include "task/Task.h"

namespace {

void handle_victim(bool is_left, float max_dist, bool push, TaskPush::Mode push_mode = TaskPush::Mode::RIGHT) {
    auto& robot = Robot::instance();
    float d = is_left ? robot.dist_left.get() : robot.dist_right.get();

    if (d > 0.0f && d < max_dist) {
        if (push) {
            robot.tasks_victim.push(TaskPush(push_mode));
        }
        robot.tasks_victim.push(TaskLed());
    } else {
        LOG_INFO("Not.", d);
    }
}

} // namespace

void Link::wait_start() const {
    for (;;) {
        serial_base->write("s\n");

        if (serial_base->available() && serial_base->read() == 's') return;
        if (serial_debug->available() && serial_debug->read() == 's') return;

        delay(100);
    }
}

void Link::update() const {
    read_from_stream(serial_base, "UART");
}

void Link::update_debug() const {
    read_from_stream(serial_debug, "GET");
}

void Link::read_from_stream(Stream* stream, const char* label) const {
    while (stream->available()) {
        char cmd = static_cast<char>(stream->read());

        Serial.print(cmd);
        Serial.print("  <- ");
        Serial.println(label);

        process_command(cmd);
    }
}

void Link::send_pause(bool paused) const {
    const char cmd = paused ? 'p' : 'o';
    serial_base->write(cmd);
    LOG_INFO(cmd);
}

void Link::process_command(char cmd) {
    auto& robot = Robot::instance();

    // Сброс состояния для активных команд
    switch (cmd) {
        case 'u': case 'l': case 'r':
        case 'v': case 'b': case 'n': case 'm':
        case 'c': case 'x': case 'e':
            robot.reset();
            break;
        default:
            break;
    }

    // Подтверждение перед началом движения
    if (cmd == 'u' || cmd == 'l' || cmd == 'r') {
        robot.tasks_move.push(TaskSent());
    }

    switch (cmd) {
        case 'u': {
            robot.tasks_move.push(TaskDelay(300));
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            robot.tasks_move.push(TaskMove(
                SpeedProfile(100, 135, Quad_MM(300), Quad_MM(5), Quad_MM(5)),
                PID(1.3, 0, 7.0, -200, 200),
                PID(1.7, 0, 7.0, -200, 200),
                PID(1.4, 0, 0.2, -90, 90)
            ));
            robot.tasks_move.push(TaskBlue());
            break;
        }
        case 'r': {
            robot.tasks_move.push(TaskRotate(90, PID(2.5, 0, 0, -200, 200)));
            break;
        }
        case 'l': {
            TaskRotate rot(90, PID(2.5, 0, 0, -200, 200));
            rot.set_direction(-1);
            robot.tasks_move.push(rot);
            break;
        }
        case 'd': {
            robot.tasks_move.push(TaskRotate(180, PID(2.5, 0, 0, -200, 200)));
            break;
        }

        // Жертвы / светодиоды
        case 'v': handle_victim(false, 130.0f, true,  TaskPush::Mode::RIGHT);    break;
        case 'b': handle_victim(true,  200.0f, true,  TaskPush::Mode::LEFT);     break;
        case 'n': handle_victim(false, 200.0f, true,  TaskPush::Mode::RIGHT_X2); break;
        case 'm': handle_victim(true,  200.0f, true,  TaskPush::Mode::LEFT_X2);  break;
        case 'c': handle_victim(false, 200.0f, false);                            break;
        case 'x': handle_victim(true,  200.0f, false);                            break;

        case 'z':
            robot.color.log();
            break;

        case 'e':
            robot.tasks_victim.push(TaskExit());
            break;

        case 'g':
            LOG_INFO("UPS: ", robot.delta_fast.get_ups());
            break;

        default:
            break;
    }
}

void Link::send_sensors(const bool distance[4], uint8_t color) const {
    const char packet[6] = {
        static_cast<char>('0' + (distance[0] ? 1 : 0)),
        static_cast<char>('0' + (distance[1] ? 1 : 0)),
        static_cast<char>('0' + (distance[2] ? 1 : 0)),
        static_cast<char>('0' + (distance[3] ? 1 : 0)),
        static_cast<char>('0' + (color <= 2 ? color : 0)),
        '\n'
    };

    serial_base->write(reinterpret_cast<const uint8_t*>(packet), sizeof(packet));
}

void Link::log_debug(const char* message) const {
    serial_debug->println(message);
}