#include "node_servo.h"

void node_servo_init(ServoNode& ctx) {
    PT_INIT(&ctx.pt);

    ctx.driver.attach(ctx.pin);

    ctx.currentPos = 0;
    ctx.last_process_time = millis();

    if (ctx.dt == 0) ctx.dt = 25;

    SUBSCRIBE_TOPIC(ctx, ctx.topic);
}

int node_servo_run(ServoNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        WAIT_NEW_DATA(ctx);
        SYNC_TOPIC(ctx, (*ctx.topic));

        ctx.move.x_start = 0;
        ctx.move.y0 = ctx.currentPos;
        ctx.move.y1 = ctx.topic->value;
        ctx.move.L_total = (float)ctx.topic->time;
        ctx.move.d_acc = ctx.topic->d_acc;

        ctx.startTime = millis();

        while (true) {
            PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_process_time) >= ctx.dt);
            ctx.last_process_time = millis();

            float elapsed = (float)(millis() - ctx.startTime);

            if (elapsed >= ctx.move.L_total || HAS_NEW_DATA(ctx, (*ctx.topic))) {
                break;
            }

            ctx.currentPos = SoftGet(&ctx.move, elapsed);
            ctx.driver.write((int)ctx.currentPos);
        }

        if (!HAS_NEW_DATA(ctx, (*ctx.topic))) {
            ctx.currentPos = ctx.move.y1;
            ctx.driver.write((int)ctx.currentPos);
        }
    }

    PT_END(&ctx.pt);
}