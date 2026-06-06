#ifndef FIRMWARE_DELTA_H
#define FIRMWARE_DELTA_H

#include <stdint.h>

class Delta {
public:
    void start();
    void stop();

    float get_ups() const { return _ups; }
    uint32_t get_dt_us() const { return _dt_us; }
    uint32_t get_exec_us() const { return _exec_us; }

private:
    uint32_t _last_start_us = 0;
    uint32_t _current_start_us = 0;
    uint32_t _dt_us = 0;
    uint32_t _exec_us = 0;

    float _ups = 0;
    uint32_t _frames = 0;
    uint32_t _last_ups_ms = 0;
};

#endif //FIRMWARE_DELTA_H