#include "node_pi.h"
#include <Arduino.h>

void node_pi_init(PI_Node& ctx) {
    PT_INIT(&ctx.pt);
    ctx.integral = 0;
    ctx.last_time = millis();
}

int node_pi_run(PI_Node& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_time) >= (uint32_t)ctx.dt);
        ctx.last_time = millis();

        float error = (float)ctx.setpoint - ctx.topic->value;

        ctx.integral += error * (ctx.dt / 1000.0f);
        ctx.integral = constrain(ctx.integral, -255.0f, 255.0f);

        float correction = (error * ctx.Kp) + (ctx.integral * ctx.Ki);

        ctx.value_out = correction;

        NOTIFY_TOPIC(ctx.topic); 
    }

    PT_END(&ctx.pt);
}
