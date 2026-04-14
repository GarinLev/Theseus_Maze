#include "node_pi.h"
#include <Arduino.h>

void node_pi_init(PI_Node& ctx) {
    PT_INIT(&ctx.pt);
    ctx.integral = 0;
    ctx.last_time = millis();

    SUBSCRIBE_TOPIC(ctx, ctx.topic);
}

int node_pi_run(PI_Node& ctx) {
    PT_BEGIN(&ctx.pt);
    while (1) {
        PT_WAIT_UNTIL(&ctx.pt, (millis() - ctx.last_time >= (uint32_t)ctx.dt));
        ctx.last_time = millis();

        float current_val = ctx.topic->value; // Убедитесь, что тут RPM, а не тики!
        float error = (float)ctx.setpoint - current_val;

        // Накопление интеграла с ограничением (Anti-Windup)
        ctx.integral += error * (ctx.dt / 1000.0f);
        ctx.integral = constrain(ctx.integral, -100, 100); // Примерные границы

        float ustav = (error * ctx.Kp) + (ctx.integral * ctx.Ki);

        // Выход — это чистый PWM
        ctx.value_out = constrain(ustav, 0, 255);
    }
    PT_END(&ctx.pt);
}
