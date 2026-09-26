#include "Link.h"
#include "Robot.h"
#include "task/Task.h"

namespace {

void push_forward_sequence(Robot& robot) {
    robot.tasks_move.push(TaskMove(
        SpeedProfile(100, 135, Quad_MM(300), Quad_MM(5), 0, 135),
        PID(0.15, 0, 0.3, -200, 200),
        PID(0.3, 0, 3.0, -200, 200),
        PID(2.5, 0, 7.5, -90, 90)
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

    if (cmd == 'k') {
        String data = "";
        uint32_t start_time = millis();
        while (millis() - start_time < 50) {
            if (serial_base->available() || serial_debug->available()) {
                char c = (serial_base->available()) ? serial_base->read() : serial_debug->read();
                if (c == '\n' || c == '\r') break;
                data += c;
            }
        }

        char buf[64];
        int len = data.length();
        if (len >= 64) len = 63;
        data.substring(0, len).toCharArray(buf, 64);

        float vals[4];
        int val_idx = 0;
        char* p_str = strtok(buf, " ,");
        while (p_str != nullptr && val_idx < 4) {
            vals[val_idx++] = atof(p_str);
            p_str = strtok(nullptr, " ,");
        }

        if (val_idx == 4) {
            robot.w_fr.set_pi_coeffs(vals[0], vals[1], vals[2], vals[3]);
            robot.w_fl.set_pi_coeffs(vals[0], vals[1], vals[2], vals[3]);
            robot.w_br.set_pi_coeffs(vals[0], vals[1], vals[2], vals[3]);
            robot.w_bl.set_pi_coeffs(vals[0], vals[1], vals[2], vals[3]);
            Serial.print("OK: a="); Serial.print(vals[0]);
            Serial.print(" b="); Serial.print(vals[1]);
            Serial.print(" c="); Serial.print(vals[2]);
            Serial.print(" d="); Serial.println(vals[3]);
            LOG_INFO("Wheel coeffs updated: a=", vals[0], " b=", vals[1], " c=", vals[2], " d=", vals[3]);
        } else {
            Serial.print("Error: "); Serial.print(cmd);
            Serial.println(" needs 4 values (a b c d)");
        }
        return;
    }

    if (cmd == 'i' || cmd == 'o' || cmd == 'p') {
        String data = "";
        uint32_t start_time = millis();
        while (millis() - start_time < 50) {
            if (serial_base->available() || serial_debug->available()) {
                char c = (serial_base->available()) ? serial_base->read() : serial_debug->read();
                if (c == '\n' || c == '\r') break;
                data += c;
            }
        }

        char buf[64];
        int len = data.length();
        if (len >= 64) len = 63;
        data.substring(0, len).toCharArray(buf, 64);

        float vals[2];
        int val_idx = 0;
        char* p_str = strtok(buf, " ,");
        while (p_str != nullptr && val_idx < 2) {
            vals[val_idx++] = atof(p_str);
            p_str = strtok(nullptr, " ,");
        }

        if (val_idx == 2) {
            int idx = (cmd == 'i') ? 0 : (cmd == 'o' ? 1 : 2);
            robot.drive_p[idx] = vals[0];
            robot.drive_d[idx] = vals[1];

            Serial.print("OK: "); Serial.print(cmd);
            Serial.print(" P="); Serial.print(vals[0]);
            Serial.print(" D="); Serial.println(vals[1]);

            LOG_INFO("PID ", (char)cmd, " updated: P=", vals[0], " D=", vals[1]);
        } else {
            Serial.print("Error: "); Serial.print(cmd);
            Serial.println(" needs 2 values (P D)");
        }
        return;
    }

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
            robot.tasks_move.push(TaskRotate(92, PID(1.5, 0, 0, -200, 200)));
            break;
        }
        case 'l': {
            robot.tasks_move.push(TaskSent());
            robot.tasks_move.push(TaskTouch(Quad_MM(50)));
            push_forward_sequence(robot);
            TaskRotate rot(92, PID(1.5, 0, 0, -200, 200));
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
