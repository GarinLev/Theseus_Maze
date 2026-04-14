#include "../../../lib/GyverIO/GyverIO.h"
#include "node_led.h"
#include "../../../lib/pt/pt.h"

void node_led_init(LedNode &ctx) {
    PT_INIT(&ctx.pt);

    gio::mode(ctx.pin, OUTPUT);

    SUBSCRIBE_TOPIC(ctx, ctx.topic);
}

int node_led_run(LedNode &ctx) {
    PT_BEGIN(&ctx.pt);
    (void)PT_YIELD_FLAG;

    for(;;) {
        WAIT_NEW_DATA(ctx);

        gio::write(ctx.pin, ctx.topic->state);
        
        SYNC_TOPIC(ctx, (*ctx.topic));
    }
    
    PT_END(&ctx.pt);
}