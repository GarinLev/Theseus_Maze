#include <Arduino.h>

#include "Robot.h"
#include "Task.h"

void TaskLed::on_init() {
    auto& robot = Robot::instance();
    robot.rpm = 0;
    robot.steer = 0;
    last_toggle = 0;
    led_on = false;
}

void TaskLed::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    if (elapsed_ms > 5000) {
        robot.led.clear();
        robot.led.show();
        done();
        return;
    }

    if (elapsed_ms - last_toggle >= 500) {
        last_toggle = elapsed_ms;
        led_on = !led_on;

        if (led_on) {
            robot.led.fill(mRed);
        } else {
            robot.led.clear();
        }
        robot.led.show();
    }
}


void TaskExit::on_init() {
    auto& robot = Robot::instance();
    robot.rpm = 0;
    robot.steer = 0;
    last_toggle = 0;
    led_on = false;
}

void TaskExit::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    if (elapsed_ms > 300000UL) {
        robot.led.clear();
        robot.led.show();
        done();
        return;
    }

    if (elapsed_ms - last_toggle >= 1000) {
        last_toggle = elapsed_ms;
        led_on = !led_on;

        if (led_on) {
            robot.led.fill(mRed);
        } else {
            robot.led.clear();
        }
        robot.led.show();
    }
}