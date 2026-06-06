#include "Task.h"
#include <Arduino.h>

void TaskDelay::on_init() {
}

void TaskDelay::on_execute(uint32_t dt) {
    if (elapsed_ms >= delay_ms) {
        done();
    }
}
