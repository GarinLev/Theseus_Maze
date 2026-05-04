#pragma once
#include "controller_wheel.h"

struct WheelManager {
    WheelController* fl;
    WheelController* fr;
    WheelController* bl;
    WheelController* br;

    void moveDistance(float mm, float target_rpm, float accel_mm = -1) {
        if (accel_mm == -1) accel_mm = 0.4f * mm;

        uint32_t ticks = (uint32_t)WC_MM_TO_TICKS(mm);
        uint32_t acc_ticks = (uint32_t)WC_MM_TO_TICKS(accel_mm);
            
        fl->goTo(target_rpm, acc_ticks, ticks);
        fr->goTo(target_rpm, acc_ticks, ticks);
        bl->goTo(target_rpm, acc_ticks, ticks);
        br->goTo(target_rpm, acc_ticks, ticks);
    }

    void stop() {
        fl->stop(); fr->stop();
        bl->stop(); br->stop();
    }

    bool isMoving() const {
        return fl->is_moving || fr->is_moving || bl->is_moving || br->is_moving;
    }

    void update() {
        fl->update();
        fr->update();
        bl->update();
        br->update();
    }
};