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
    // 1. Защита от некорректных параметров (цепочка 0 <= lu <= ld <= l)
    // Используем встроенные макросы Arduino вместо std::max
    float valid_lu = max(0.0f, lu);
    float valid_ld = max(valid_lu, ld);
    float valid_l  = max(valid_ld, l);

    float sign = (ln >= 0.0f) ? 1.0f : -1.0f;
    float abs_ln = fabsf(ln);
    float speed = ss;

    // Малое значение для исключения деления на околонулевые величины
    // Для 32-битного float на Arduino 1e-4f — оптимальный порог
    const float EPSILON = 0.0001f; 

    // 2. Участок 1: Разгон (0 <= abs_ln < valid_lu)
    if (abs_ln < valid_lu) {
        if (valid_lu < EPSILON) {
            speed = su; // Если разгон почти нулевой длины, сразу выдаем маршевую скорость
        } else {
            float t = abs_ln / valid_lu;
            speed = ss + (su - ss) * smoothStep(t);
        }
    } 
    // 3. Участок 2: Движение с постоянной скоростью (valid_lu <= abs_ln < valid_ld)
    else if (abs_ln >= valid_lu && abs_ln < valid_ld) {
        speed = su;
    } 
    // 4. Участок 3: Торможение (valid_ld <= abs_ln)
    else if (abs_ln >= valid_ld) {
        float total_deceleration_len = valid_l - valid_ld;

        if (total_deceleration_len < EPSILON || abs_ln >= valid_l) {
            speed = se; // Если тормозить негде или точка финиша пройдена, выдаем конечную скорость se
        } else {
            float t = (abs_ln - valid_ld) / total_deceleration_len;
            // Плавно снижаем скорость от su до se
            speed = su + (se - su) * smoothStep(t);
        }
    }

    return speed * sign;
}
