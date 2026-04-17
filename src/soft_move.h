#pragma once
#include <Arduino.h>

struct SoftMove {
    float x_start;
    float L_total;
    float y0, y1;
    float d_acc;
};

inline float SoftGet(SoftMove* state, float current_x) {
    if (!state) return 0.0f;

    float x = abs(current_x - state->x_start);
    float L = state->L_total;
    float d = state->d_acc;

    if (x <= 0) return state->y0;
    if (x >= L) return state->y0;

    float y_max = state->y1;
    if (L < 2.0f * d) {
        float mid_ratio = (L / 2.0f) / d;
        float s_mid = sin(0.5f * PI * mid_ratio);
        y_max = (state->y1 - state->y0) * (s_mid * s_mid) + state->y0;
    }

    float x1 = (L < 2.0f * d) ? L / 2.0f : d;
    float x2 = L - x1;

    if (x < x1) {
        float ratio = x / x1;
        float s = sin(0.5f * PI * ratio);
        return (y_max - state->y0) * (s * s) + state->y0;
    }
    else if (x < x2) {
        return y_max;
    }
    else {
        float ratio = (x - x2) / x1;
        float s = sin(0.5f * PI * ratio);
        return (state->y0 - y_max) * (s * s) + y_max;
    }
}