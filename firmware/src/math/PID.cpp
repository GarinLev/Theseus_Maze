#include <math.h>
#include "PID.h"

float PID::compute(const float setpoint, const float process_value) {
    last_setpoint = setpoint;

    if (isnan(process_value) || isnan(setpoint)) {
        last_error = 0.0f;
        return out_min;
    }

    const float error = setpoint - process_value;
    last_error = error;

    const float p_term = Kp * error;

    integrator += error;

    float i_term = 0.0f;
    if (Ki != 0.0f) {
        i_term = Ki * integrator;

        if (i_term > out_max) {
            i_term = out_max;
            integrator = out_max / Ki;
        } else if (i_term < out_min) {
            i_term = out_min;
            integrator = out_min / Ki;
        }
    } else {
        integrator = 0.0f;
    }

    const float d_term = Kd * (error - prev_error);
    prev_error = error;

    float output = p_term + i_term + d_term;

    if (output > out_max) {
        output = out_max;
    } else if (output < out_min) {
        output = out_min;
    }

    return output;
}

void PID::reset() {
    integrator = 0.0f;
    prev_error = 0.0f;
    last_error = 0.0f;
    last_setpoint = 0.0f;
}