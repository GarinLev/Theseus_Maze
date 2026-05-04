#include "node_servo.h"

void node_servo_init(ServoNode& ctx, uint8_t pin, float init_angle) {
    PT_INIT(&ctx.pt);
    ctx.pin = pin;
    ctx.motor.attach(pin);
    ctx.current_angle = init_angle;
    ctx.motor.write((int)init_angle);
    ctx.has_new_task = false;
}

int node_servo_run(ServoNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt, ctx.has_new_task);
        ctx.has_new_task = false;

        while (millis() < ctx.end_time) {
            PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_update) >= 50);

            ctx.last_update = millis();

            ctx.current_angle = SoftSetGet(&ctx.movement, (float)ctx.last_update);
            ctx.motor.write((int)ctx.current_angle);
        }

        ctx.current_angle = ctx.movement.y1;
        ctx.motor.write((int)ctx.current_angle);
    }

    PT_END(&ctx.pt);
}