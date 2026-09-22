#include "Link.h"
#include "Robot.h"
#include "task/Task.h"

namespace {

void push_forward_sequence(Robot& robot) {
    robot.tasks_move.push(TaskMove(
        SpeedProfile(100, 135, Quad_MM(300), Quad_MM(5), 0, 135),
        PID(0.3, 0, 3.0, -200, 200),
        PID(0.7, 0, 3.0, -200, 200),
        PID(1.4, 0, 0.2, -90, 90)
    ));
}

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

}

void Link::wait_start() const {
    for (;;) {
        serial_base->write("s\n");

        if (serial_base->available() && serial_base->read() == 's') return;
        if (serial_debug->available() && serial_debug->read() == 's') return;

        delay(100);
    }
}

void Link::update() {
    read_from_stream(serial_base, "UART");
}

void Link::update_debug() {
    read_from_stream(serial_debug, "GET");
}

void Link::read_from_stream(Stream* stream, const char* label) {
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

    if (cmd == 'u' || cmd == 'l' || cmd == 'r' || cmd == 'd') {
        if (!robot.tasks_move.isEmpty()) {
            command_queue[queue_tail] = cmd;
            queue_tail = (queue_tail + 1) % 16;
            return;
        }
    }
    execute_command(cmd);
}

void Link::execute_command(char cmd) {
    auto& robot = Robot::instance();

    // Сброс состояния для активных команд
    switch (cmd) {
        case 'l': case 'r':
        case 'd':
        case 'v': case 'b': case 'n': case 'm':
        case 'c': case 'x': case 'e':
            robot.reset();
            robot.force_stop = true;
            break;
        default:
            break;
    }

    switch (cmd) {
        case 'u': {
            robot.tasks_move.push(TaskSent());
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            push_forward_sequence(robot);
            robot.tasks_move.push(TaskBlue());
            break;
        }
        case 'r': {
            robot.tasks_move.push(TaskSent());
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            push_forward_sequence(robot);
            robot.tasks_move.push(TaskRotate(90, PID(2.5, 0, 0, -200, 200)));
            break;
        }
        case 'l': {
            robot.tasks_move.push(TaskSent());
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            push_forward_sequence(robot);
            TaskRotate rot(90, PID(2.5, 0, 0, -200, 200));
            rot.set_direction(-1);
            robot.tasks_move.push(rot);
            break;
        }
        case 'd': {
            robot.tasks_move.push(TaskSent());
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            push_forward_sequence(robot);
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

void Link::update_command_queue() {
    auto& robot = Robot::instance();
    if (robot.tasks_move.isEmpty() && queue_head != queue_tail) {
        char cmd = command_queue[queue_head];
        queue_head = (queue_head + 1) % 16;
        execute_command(cmd);
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
    serial_debug->write(reinterpret_cast<const uint8_t*>(packet), sizeof(packet));

}

void Link::log_debug(const char* message) const {
    serial_debug->println(message);
}