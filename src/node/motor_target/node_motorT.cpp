#include "../../../lib/GyverIO/GyverIO.h"
#include "node_motorT.h"

#define SPEED_MAX 255
#define SPEED_MIN_SOFT 70
#define SPEED_MIN 0

void node_motorT_init(MotorTargetNode &ctx) {
    PT_INIT(&ctx.pt);

    ctx.soft.x0 = 0;
    ctx.soft.x1 = 0;
    ctx.soft.a = 20;
    ctx.soft.Vmax = 50;
}


int node_motorT_run(MotorTargetNode& ctx, uint16_t x) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_time) >= 25);
        ctx.last_time = millis();

        float value = SoftGet(&ctx.soft, x);

        ctx.topic_out->speed = value;
        NOTIFY_TOPIC(ctx.topic_out);
    }

    PT_END(&ctx.pt);
}
