#include "Link.h"
#include "Robot.h"
#include "task/Task.h"
#include <Arduino.h>


void Link::wait_start() const {
    for (;;) {
        serial_base->write('s');

        if (serial_base->available() || serial_debug->available()) {
            int16_t c = serial_base->read();
            int16_t d = serial_debug->read();
            if (c == 's' || d == 's') { return; }
        }
        delay(100);
    }
}

void Link::update() const {
    if (serial_base->available()) {
        char cmd = static_cast<char>(serial_base->read());
        process_command(cmd);
    }
}

void Link::update_debug() const {
    if (serial_debug->available()) {
        char cmd = static_cast<char>(serial_debug->read());
        Serial.println("  <- GET");
        process_command(cmd);
    }
}

void Link::process_command(char cmd) {
    auto& robot = Robot::instance();

    if (cmd == 'u' || cmd == 'd' || cmd == 'l' || cmd == 'r' || 
        cmd == 'v' || cmd == 'b' || cmd == 'n' || cmd == 'm' || cmd == 'c') {
        robot.reset();
    }

    if (cmd == 'u' || cmd == 'd' || cmd == 'l' || cmd == 'r') {
        robot.tasks.push( TaskSent() );
        robot.tasks.push(TaskDelay(1000));
        robot.tasks.push(TaskTouch(Quad_MM(50)));
        robot.tasks.push(TaskMove(
            SpeedProfile(30, 100, Quad_MM(300), Quad_MM(200), Quad_MM(50)),
            PID(0.15, 0, 0.04, -200, 200),
            PID(1.1, 0, 0.1, -30, 30)
        ));
        robot.tasks.push(TaskBlue());
    }

    switch (cmd) {
        case 'u': break;
        case 'd': {
            robot.tasks.push(TaskDelay(500));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 85, 30, 30)));
            robot.tasks.push(TaskDelay(500));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 85, 30, 30)));
            break;
        }
        case 'r': {      // Право
            robot.tasks.push(TaskDelay(500));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 85, 30, 30)));
            break;
        }
        case 'l': {      // Лево
            robot.tasks.push(TaskDelay(500));
            auto rot = TaskRotate(SpeedProfile(30, 60, 85, 30, 30));
            rot.set_direction(-1);
            robot.tasks.push(rot);
            break;
        }
        case 'v': {
            robot.tasks.push(TaskPush(TaskPush::Mode::RIGHT));
            robot.tasks.push(TaskLed());
            break;
        }
        case 'b': {
            robot.tasks.push(TaskPush(TaskPush::Mode::LEFT));
            robot.tasks.push(TaskLed());
            break;
        }
        case 'n': {
            robot.tasks.push(TaskPush(TaskPush::Mode::RIGHT_X2));
            robot.tasks.push(TaskLed());
            break;
        }
        case 'm': {
            robot.tasks.push(TaskPush(TaskPush::Mode::LEFT_X2));
            robot.tasks.push(TaskLed());
            break;
        }
        case 'c': {
            robot.tasks.push(TaskLed());
            break;
        }
        case 'z': {
            robot.color.log();
            break;
        }
        case 'g': {
            LOG_INFO("UPS: ", robot.delta_fast.get_ups());
            break;
        }
        default:
            break;
    }
}

void Link::send_sensors(const bool distance[4], uint8_t color) const {
    char packet[6];

    packet[0] = static_cast<char>('0' + (distance[0] ? 1 : 0));
    packet[1] = static_cast<char>('0' + (distance[1] ? 1 : 0));
    packet[2] = static_cast<char>('0' + (distance[2] ? 1 : 0));
    packet[3] = static_cast<char>('0' + (distance[3] ? 1 : 0));
    packet[4] = static_cast<char>('0' + (color <= 2 ? color : 0));
    packet[5] = '\0';

    serial_base->write((uint8_t*)packet, 5);

    serial_debug->print("[SENSORS SENT]: ");
    serial_debug->println(packet);
}

void Link::log_debug(const char* message) const {
    serial_debug->println(message);
}