#include <math.h>
#include "SpeedProfile.h"

// Переносим константу сюда, чтобы компилятор её видел
constexpr float Eu_K = 0.6f;

namespace {
float Eu(float t) {
    float t_minus_1 = t - 1.0f;
    float cube = t_minus_1 * t_minus_1 * t_minus_1;
    float square = t_minus_1 * t_minus_1;

    return 1.0f + (Eu_K + 1.0f) * cube + Eu_K * square;
}

float Ed(float t) {
    float t2 = t * t;
    return t2 * t2 * t; // t^5
}
}

float SpeedProfile::compute(float ln) const {
    float sign = (ln >= 0.0f) ? 1.0f : -1.0f;
    float abs_ln = fabsf(ln);

    float speed = ss;

    if (abs_ln < lu) {
        if (lu <= 0.0f) return su * sign;

        float t = abs_ln / lu;
        if (t > 1.0f) t = 1.0f; // Защита, чтобы t не вылетало за [0, 1]

        speed = ss + (su - ss) * Eu(t);
    }
    else if (abs_ln >= lu && abs_ln < ld) {
        speed = su;
    }
    else if (abs_ln >= ld) {
        float total_deceleration_len = l - ld;
        if (total_deceleration_len <= 0.0f) return ss * sign;

        float t = (abs_ln - ld) / total_deceleration_len;
        if (t > 1.0f) t = 1.0f; // Защита от перелета

        speed = su + (ss - su) * Ed(t);
    }

    return speed * sign;
}