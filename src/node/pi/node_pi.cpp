#include "node_pi.h"
#include <Arduino.h>

void node_pi_init(PI_Node& ctx) {
    PT_INIT(&ctx.pt);
    ctx.integral = 0;
    ctx.last_time = millis();
}

int node_pi_run(PI_Node& ctx, float* input_value) {
    PT_BEGIN(&ctx.pt);

    while (1) {
        PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_time) >= ctx.dt);

        ctx.last_time = millis();

        float current_val = *input_value;
        float error = (float)ctx.setpoint - current_val;

        ctx.integral += error * (ctx.dt / 1000.0f);

        float ustav = (error * ctx.Kp) + (ctx.integral * ctx.Ki);

        ctx.topic->value = ustav;
        NOTIFY_TOPIC(ctx.topic);
    }

    PT_END(&ctx.pt);
}
