#pragma once

#include "controller_wall.h"
#include "controller_wheel.h"

struct WallManagerController {
    WallController* sens_right;
    WallController* sens_left;

    WheelController* wheelA1;
    WheelController* wheelA2;
    WheelController* wheelB1;
    WheelController* wheelB2;

    void update() {
        uint16_t offset;

        if (sens_right->offset.hasValue()) {
            offset = sens_right->offset.getValue();
        } else if (sens_left->offset.hasValue()) {
            offset = sens_left->offset.getValue();
        } else {
            offset = 0;
            return;
        }

        wheelA1->speed_offset = offset;
        wheelA2->speed_offset = offset;
        wheelB1->speed_offset = offset * -1;
        wheelB2->speed_offset = offset * -1;
    }
};
