#include <Arduino.h>

#include "Log.h"
#include "Robot.h"
#include "Task.h"

void TaskBlue::on_init() {
    auto& robot = Robot::instance();

    if (robot.color.get_current_color() != COLOR_BLUE) {
        active = false;
        return;
    }

    active = true;
    blue_wait_start = millis();
}

void TaskBlue::on_execute() {
    if (!active) {
        done();
        return;
    }

    if (millis() - blue_wait_start > 5000) {
        done();
    }
}
