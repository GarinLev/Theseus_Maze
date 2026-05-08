#pragma once
#include <Arduino.h>


struct SoftSet {
    float x0, x1;
    float y0, y1;
};

inline float SoftSetGet(SoftSet* state, float current_x) {
    if (!state) return 0.0f;

    float total_dist = state->x1 - state->x0;

    float current_dist = current_x - state->x0;

    if (total_dist == 0) return state->y1;

    float ratio = current_dist / total_dist;

    if (ratio <= 0.0f) return state->y0;
    if (ratio >= 1.0f) return state->y1;


    float s = sin(0.5f * PI * ratio);
    float smooth_step = s * s;

    return state->y0 + (state->y1 - state->y0) * smooth_step;
}