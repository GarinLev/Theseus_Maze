#include "Quad.h"
#include <Arduino.h>

void Quad::update(float fr_target, float fl_target, float br_target, float bl_target) const {
    static float smoothed_sync_target = 0.0f;

    float real_fr = fr ? fr->real() : 0.0f;
    float real_fl = fl ? fl->real() : 0.0f;
    float real_br = br ? br->real() : 0.0f;
    float real_bl = bl ? bl->real() : 0.0f;

    if (fr_target == 0.0f && fl_target == 0.0f && br_target == 0.0f && bl_target == 0.0f) {
        if (fr) fr->update(0.0f);
        if (fl) fl->update(0.0f);
        if (br) br->update(0.0f);
        if (bl) bl->update(0.0f);

        smoothed_sync_target = 0;

        return;
    }

    float speeds[] = {real_fr, real_fl, real_br, real_bl};
    float targets[] = {fr_target, fl_target, br_target, bl_target};
    bool active[] = {fr != nullptr, fl != nullptr, br != nullptr, bl != nullptr};

    float sum_target = 0.0f;
    int active_wheels = 0;

    for (int i = 0; i < 4; i++) {
        if (active[i]) {
            sum_target += targets[i];
            active_wheels++;
        }
    }

    if (active_wheels == 0) return;

    float avg_target = sum_target / active_wheels;
    bool is_reverse = (avg_target < 0.0f);

    float sum_norm_speed = 0.0f;
    float worst_norm_speed = is_reverse ? -1e6f : 1e6f;
    float norm_speeds[4] = {0.0f};

    for (int i = 0; i < 4; i++) {
        if (active[i]) {
            float target_offset = targets[i] - avg_target;
            norm_speeds[i] = speeds[i] - target_offset;
            sum_norm_speed += norm_speeds[i];

            if (is_reverse) {
                if (norm_speeds[i] > worst_norm_speed) worst_norm_speed = norm_speeds[i];
            } else {
                if (norm_speeds[i] < worst_norm_speed) worst_norm_speed = norm_speeds[i];
            }
        }
    }

    float avg_norm_speed = sum_norm_speed / active_wheels;
    float raw_sync_target = (avg_norm_speed * 0.5f) + (worst_norm_speed * 0.5f);

    if ((raw_sync_target > 0.0f && smoothed_sync_target < 0.0f) ||
        (raw_sync_target < 0.0f && smoothed_sync_target > 0.0f)) {
        smoothed_sync_target = raw_sync_target;
    } else {
        smoothed_sync_target = (smoothed_sync_target * 0.85f) + (raw_sync_target * 0.15f);
    }

    float fr_cmd = fr_target;
    float fl_cmd = fl_target;
    float br_cmd = br_target;
    float bl_cmd = bl_target;

    if (fr) fr->update(fr_cmd);
    if (fl) fl->update(fl_cmd);
    if (br) br->update(br_cmd);
    if (bl) bl->update(bl_cmd);
}

float Quad::encoder() const {
    float sum = 0;
    if (fr) sum += fr->get_encoder();
    if (fl) sum += fl->get_encoder();
    if (br) sum += br->get_encoder();
    if (bl) sum += bl->get_encoder();
    return sum / 4.0f;
}

void Quad::encoder_reset() const {
    if (fr) const_cast<Wheel*>(fr)->reset_encoder();
    if (fl) const_cast<Wheel*>(fl)->reset_encoder();
    if (br) const_cast<Wheel*>(br)->reset_encoder();
    if (bl) const_cast<Wheel*>(bl)->reset_encoder();
}

void Quad::rpm(float target_rpm, float steer) const {
    float fr_target = target_rpm + steer;
    float fl_target = target_rpm + steer;
    float br_target = target_rpm - steer;
    float bl_target = target_rpm - steer;
    update(fr_target, fl_target, br_target, bl_target);
}
