#include "Task.h"
#include <Arduino.h>

void TaskDelay::on_init() {
    start_time = millis();
}

void TaskDelay::on_execute() {
    if (millis() - start_time >= delay_ms) {
        done();
    }
}
