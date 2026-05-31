#include <math.h>

#include "SpeedProfile.h"

constexpr float Eu_K = 0.6f;

namespace {
float Eu(float t) {
    return 1 + (Eu_K + 1) * pow(t - 1, 3) + Eu_K * pow(t - 1, 2);
}

float Ed(float t) {
    return pow(t, 5);
}
}

float SpeedProfile::compute(float ln) const {
    float sign = (ln >= 0.0f) ? 1.0f : -1.0f;
    float abs_ln = fabsf(ln);

    float speed = ss;

    if (abs_ln < lu) {
        if (lu <= 0.0f) return su * sign;

        float t = abs_ln / lu;
        speed = ss + (su - ss) * Eu(t);
    }
    else if (abs_ln >= lu && abs_ln < ld) {
        speed = su;
    }
    else if (abs_ln >= ld) {
        float total_deceleration_len = l - ld;
        if (total_deceleration_len <= 0.0f) return ss * sign;

        float t = (abs_ln - ld) / total_deceleration_len;
        if (t > 1.0f) t = 1.0f;
        speed = su + (ss - su) * Ed(t);
    }

    return speed * sign;
}
