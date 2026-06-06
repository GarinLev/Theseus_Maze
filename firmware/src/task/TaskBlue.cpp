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
}

void TaskBlue::on_execute(uint32_t dt) {
    if (!active) {
        done();
        return;
    }

    if (elapsed_ms > 5000) {
        done();
    }
}
