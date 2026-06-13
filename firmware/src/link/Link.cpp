#include "Link.h"
#include "Robot.h"
#include "Log.h"
#include <Arduino.h>

void Link::init() const {
    serial->begin(9600);
}

void Link::wait_start() const {
    for (;;) {
        serial->write('s');
        LOG_TRACE("Sent status: s");

        if (serial->available()) {
            char c = serial->read();
            if (c == 's') { return; }
        }
        delay(100);
    }
}

void Link::update() const {
    auto& robot = Robot::instance();

    char cmd = serial->read();

    if (cmd == 'u' || cmd == 'd' || cmd == 'l' || cmd == 'r') {
        robot.tasks.push( TaskSent() );
        robot.tasks.push(TaskDelay(1000));
        robot.tasks.push(TaskTouch(Quad_MM(50)));
        robot.tasks.push(TaskMove(
            SpeedProfile(30, 100, Quad_MM(300), Quad_MM(200), Quad_MM(50)),
            PID(0.15, 0, 0.04, -200, 200),
            PID(1.1, 0, 0.1, -30, 30)
        ));
    }

    switch (cmd) {
        case 'u': break;
        case 'd': {
            robot.tasks.push(TaskDelay(1000));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 90, 30, 30)));
            robot.tasks.push(TaskDelay(1000));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 90, 30, 30)));
            break;
        }
        case 'r': {      // Право
            robot.tasks.push(TaskDelay(1000));
            robot.tasks.push(TaskRotate(SpeedProfile(30, 60, 90, 30, 30)));
            break;
        }
        case 'l': {      // Лево
            robot.tasks.push(TaskDelay(1000));
            auto rot = TaskRotate(SpeedProfile(30, 60, 90, 30, 30));
            rot.set_direction(-1);
            robot.tasks.push(rot);
            break;
        }

        case 'v': /* одна на право */ break;
        case 'b': /* одна на лево */ break;
        case 'n': /* две на право */ break;
        case 'm': /* две на лево */ break;
        case 'c': /* просто мигать */ break;

        case 'e': {
            while(!robot.tasks.isEmpty()) robot.tasks.pop();
            break;
        }
        default:
            break;
    }
}

void Link::send_sensors(bool distance[4], uint8_t color) const {
    char packet[5];

    packet[0] = '0' + (distance[0] ? 1 : 0); // Передний (vl)
    packet[1] = '0' + (distance[1] ? 1 : 0); // Правый
    packet[2] = '0' + (distance[2] ? 1 : 0); // Задний
    packet[3] = '0' + (distance[3] ? 1 : 0); // Левый
    packet[4] = '0' + (color <= 2 ? color : 0); // Цвет клетки

    serial->write((uint8_t*)packet, 5);
}