#pragma once
#include <Arduino.h>
#include "../../lib/pt/pt.h"

struct DebugController {
    struct pt pt_task;
    uint32_t timer = 0;

    struct pt pt_ups;
    uint32_t ups_timer;
    uint32_t frame_count;
    float ups_last = 0;



    void init() {
        PT_INIT(&pt_task);
    }

    int ups_update()
    {
        PT_BEGIN(&pt_ups);

        for (;;) {
            frame_count = 0;
            ups_timer = millis();

            PT_WAIT_UNTIL(&pt_ups, (uint32_t)(millis() - ups_timer) >= 1000);

            ups_last = frame_count;
        }

        PT_END(&pt_ups);
    }
    void frame() {
        frame_count += 1;
    }


    int update();
    void print();
};
