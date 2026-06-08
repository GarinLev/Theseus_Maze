#include "Delta.h"
#include <Arduino.h>

void Delta::start() {
    uint32_t now_us = micros();
    uint32_t now_ms = millis();

    if (_last_start_us == 0) {
        _last_start_us = now_us;
        _last_ups_ms = now_ms;
    }

    _dt_us = now_us - _last_start_us;
    _last_start_us = now_us;
    _current_start_us = now_us;

    _frames++;
    if (now_ms - _last_ups_ms >= 1000) {
        _ups = (float)_frames * 1000.0f / (float)(now_ms - _last_ups_ms);
        _last_ups_ms = now_ms;
        _frames = 0;
    }
}

void Delta::stop() {
    _exec_us = micros() - _current_start_us;
}