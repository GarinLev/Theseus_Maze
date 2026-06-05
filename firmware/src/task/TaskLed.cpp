#include <Arduino.h>

#include "Robot.h"
#include "Task.h"

void TaskLed::on_init() {
    auto& robot = Robot::instance();
    robot.rpm = 0;
    robot.steer = 0;
    start_time = millis();
    last_toggle = start_time;
    led_on = false;
}

void TaskLed::on_execute() {
    auto& robot = Robot::instance();
    uint32_t now = millis();

    if (now - start_time > 5000) {
        robot.led.clear();
        robot.led.show();
        done();
        return;
    }

    if (now - last_toggle >= 500) {
        last_toggle = now;
        led_on = !led_on;

        if (led_on) {
            robot.led.fill(mRed);
        } else {
            robot.led.clear();
        }
        robot.led.show();
    }
}
