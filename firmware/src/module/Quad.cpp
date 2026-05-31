#include "Quad.h"

void Quad::update(float fr_target, float fl_target, float br_target, float bl_target) const {
    static float smoothed_sync_target = 0.0f;

    if (fr_target == 0.0f && fl_target == 0.0f && br_target == 0.0f && bl_target == 0.0f) {
        if (fr) fr->update(0.0f);
        if (fl) fl->update(0.0f);
        if (br) br->update(0.0f);
        if (bl) bl->update(0.0f);

        smoothed_sync_target = 0;
        return;
    }

    float real_fr = fr ? fr->real() : 0.0f;
    float real_fl = fl ? fl->real() : 0.0f;
    float real_br = br ? br->real() : 0.0f;
    float real_bl = bl ? bl->real() : 0.0f;

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

    float sum_norm_speed = 0.0f;
    float min_norm_speed = 1e6f;
    float norm_speeds[4] = {0.0f};

    for (int i = 0; i < 4; i++) {
        if (active[i]) {
            float target_offset = targets[i] - avg_target;

            norm_speeds[i] = speeds[i] - target_offset;

            sum_norm_speed += norm_speeds[i];
            if (norm_speeds[i] < min_norm_speed) {
                min_norm_speed = norm_speeds[i];
            }
        }
    }

    float avg_norm_speed = sum_norm_speed / active_wheels;
    float raw_sync_target = (avg_norm_speed * 0.3f) + (min_norm_speed * 0.7f);

    smoothed_sync_target = (smoothed_sync_target * 0.8f) + (raw_sync_target * 0.2f);

    constexpr float K_sync = 0.6f;

    if (fr) fr->update(fr_target + K_sync * (smoothed_sync_target - norm_speeds[0]));
    if (fl) fl->update(fl_target + K_sync * (smoothed_sync_target - norm_speeds[1]));
    if (br) br->update(br_target + K_sync * (smoothed_sync_target - norm_speeds[2]));
    if (bl) bl->update(bl_target + K_sync * (smoothed_sync_target - norm_speeds[3]));
}


void Quad::rpm(float rpm, float steer) const {
    update(
        rpm + steer, rpm + steer,
        rpm - steer, rpm - steer);
}

float Quad::encoder() const {
    int32_t fr_val = fr ? fr->get_encoder() : 0;
    int32_t fl_val = fl ? fl->get_encoder() : 0;
    int32_t br_val = br ? br->get_encoder() : 0;
    int32_t bl_val = bl ? bl->get_encoder() : 0;

    return (fr_val + fl_val + br_val + bl_val) / 4;
}

void Quad::encoder_reset() const {
    if (fr) fr->reset_encoder();
    if (fl) fl->reset_encoder();
    if (br) br->reset_encoder();
    if (bl) bl->reset_encoder();
}
