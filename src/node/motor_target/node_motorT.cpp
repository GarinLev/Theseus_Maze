#include "node_motorT.h"

void node_motorT_init(MotorTargetNode& ctx) {
    PT_INIT(&ctx.pt);
    ctx.last_time = millis();
}

float node_motorT_step(MotorTargetNode& ctx, uint32_t x) {
    return SoftGet(&ctx.soft, (float)x);
}