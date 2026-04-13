#pragma once
#include <Arduino.h>

struct SoftMove {
    float x0, x1;
    float a, Vmax;
    unsigned long startTime;
};

inline int SoftSign(float val) {
    return (0.0f < val) - (val < 0.0f);
}

inline float SoftGet(SoftMove* state) {
    if (!state) return 0.0f;

    float x = (millis() - state->startTime) / 1000.0f;
    float t0 = 0.0f;

    const float x0 = state->x0;
    const float x1 = state->x1;
    const float a = state->a;
    const float Vmax = state->Vmax;

    const float abs_diff = abs(x1 - x0);
    const int s = SoftSign(x1 - x0);

    const float sqrt_val = sqrt(abs_diff / a);

    const float t5 = sqrt_val;
    const float t4 = 2.0f * sqrt_val;
    const float t3 = abs_diff / Vmax;
    const float t2 = Vmax / a;
    const float t1 = t2 + t3;

    const float Vmax2 = a * t5;

    float dt;

    if (x >= t0 && x <= min(t2, t5)) {
        dt = x - t0;
        float val = (a * dt * dt) / 2.0f;
        return x0 + val * s;
    }

    if (x >= t5 && x <= t4) {
        float term1 = (a * t5 * t5) / 2.0f;
        float term2 = Vmax2 * (x - t5);
        dt = x - t5;
        float term3 = (a * dt * dt) / 2.0f;

        float val = term1 + term2 - term3;
        return x0 + val * s;
    }

    if (x >= t2 && x <= t3) {
        float val = ((Vmax * Vmax) / (2.0f * a)) + Vmax * (x - t2);
        return x0 + val * s;
    }

    if (x >= t3 && x <= t1) {
        float term1 = (Vmax * Vmax) / (2.0f * a);
        float term2 = Vmax * (t3 - t2);
        float term3 = Vmax * (x - t3);
        dt = x - t3;
        float term4 = (a * dt * dt) / 2.0f;

        float val = term1 + term2 + term3 - term4;
        return x0 + val * s;
    }

    return (x > t1 || x > t4) ? x1 : x0;
}
