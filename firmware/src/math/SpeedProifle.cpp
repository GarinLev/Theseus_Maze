#include <math.h>
#include "SpeedProfile.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))

namespace {
    // Оптимизированная функция плавного S-перехода (Smoothstep)
    // t принимает значения строго от 0.0 до 1.0
    float smoothStep(float t) {
        if (t <= 0.0f) return 0.0f;
        if (t >= 1.0f) return 1.0f;
        return t * t * (3.0f - 2.0f * t);
    }
}

float SpeedProfile::compute(float ln) const {
    // 1. Знак определяется направлением движения всего профиля (l),
    // а не мгновенным шумом энкодера около нуля.
    float sign = (l >= 0.0f) ? 1.0f : -1.0f;

    // 2. Исключаем движение в противоположную сторону из-за люфта/отката на старте
    float progress = ln;
    if (l >= 0.0f && progress < 0.0f) {
        progress = 0.0f;
    } else if (l < 0.0f && progress > 0.0f) {
        progress = 0.0f;
    }

    float abs_ln = fabsf(progress);
    float valid_lu = max(0.0f, fabsf(lu));
    float valid_ld = max(valid_lu, fabsf(ld));
    float valid_l  = max(valid_ld, fabsf(l));

    float speed = ss;
    const float EPSILON = 0.0001f;

    // 3. Участок 1: Разгон (0 <= abs_ln < valid_lu)
    if (abs_ln < valid_lu) {
        if (valid_lu < EPSILON) {
            speed = su;
        } else {
            float t = abs_ln / valid_lu;
            speed = ss + (su - ss) * smoothStep(t);
        }
    } 
    // 4. Участок 2: Движение с постоянной скоростью (valid_lu <= abs_ln < valid_ld)
    else if (abs_ln >= valid_lu && abs_ln < valid_ld) {
        speed = su;
    } 
    // 5. Участок 3: Торможение (valid_ld <= abs_ln)
    else {
        float total_deceleration_len = valid_l - valid_ld;

        if (total_deceleration_len < EPSILON || abs_ln >= valid_l) {
            speed = se;
        } else {
            float t = (abs_ln - valid_ld) / total_deceleration_len;
            speed = su + (se - su) * smoothStep(t);
        }
    }

    return speed * sign;
}