#include "../../../lib/GyverIO/GyverIO.h"
#include "node_dist.h"
#include "../../../lib/pt/pt.h"
#include "../../../lib/AceSorting/AceSorting.h"

#define MAX_VALID_RANGE 230

void node_dist_init(DistNode& ctx) {
    PT_INIT(&ctx.pt);

    gio::mode(ctx.pin_sht, OUTPUT);
    gio::write(ctx.pin_sht, LOW);
    delay(10);
    gio::write(ctx.pin_sht, HIGH);
    delay(10);

    ctx.lox.setBus(ctx.wire);
    if (!ctx.lox.init()) {
        Serial.println(F("Failed to boot VL53L0X"));
        for (;;);
    }

    ctx.lox.setAddress(ctx.addr);
    ctx.lox.setTimeout(0);


    ctx.lox.startContinuous();
}

int node_dist_run(DistNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        uint16_t current_range;

        PT_WAIT_UNTIL(&ctx.pt, ctx.lox.readRangeNoBlocking(current_range));

        if (current_range < MAX_VALID_RANGE && !ctx.lox.timeoutOccurred()) {
            ctx.dist_arr_buff[ctx.dist_arr_idx] = current_range;

            if (++ctx.dist_arr_idx >= DIST_ARRAY_LEN) {
                ctx.dist_arr_idx = 0;
            }
            ctx.outOfRange = false;
        }
        else {
            ctx.outOfRange = true;
        }

        if (!ctx.outOfRange) {
            uint16_t dist_arr_sort[DIST_ARRAY_LEN];
            memcpy(dist_arr_sort, ctx.dist_arr_buff, sizeof(dist_arr_sort));

            ace_sorting::shellSortKnuth(dist_arr_sort, DIST_ARRAY_LEN);

            ctx.dist_out = dist_arr_sort[DIST_ARRAY_LEN / 2];
        }

        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}