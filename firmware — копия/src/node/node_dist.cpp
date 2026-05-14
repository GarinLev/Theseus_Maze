#include "../../lib/GyverIO/GyverIO.h"
#include "node_dist.h"
#include "../../lib/pt/pt.h"
#include "../../lib/AceSorting/AceSorting.h"

#define MAX_VALID_RANGE 8000

void node_dist_init(DistNode& ctx) {
    PT_INIT(&ctx.pt);

    gio::mode(ctx.pin_sht, OUTPUT);

    node_dist_reset(ctx);
}

void node_dist_unreset(DistNode& ctx) {
    gio::write(ctx.pin_sht, HIGH);
    delay(10);
}

void node_dist_reset(DistNode& ctx) {
    gio::write(ctx.pin_sht, LOW);
    delay(10);
}

int node_dist_run(DistNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_read) >= 10);
        ctx.last_read = millis();

        uint8_t status = ctx.lox.readReg(VL53L0X_mod::RESULT_INTERRUPT_STATUS);
        if (status & 0x07) {
            uint16_t range = ctx.lox.readReg16Bit(VL53L0X_mod::RESULT_RANGE_STATUS + 10);
            ctx.lox.writeReg(VL53L0X_mod::SYSTEM_INTERRUPT_CLEAR, 0x01);

            if (range < MAX_VALID_RANGE) {

                ctx.dist_arr_buff[ctx.dist_arr_idx] = range;
                if (++ctx.dist_arr_idx >= DIST_ARRAY_LEN) {
                    ctx.dist_arr_idx = 0;
                }

                uint16_t dist_arr_sort[DIST_ARRAY_LEN];
                memcpy(dist_arr_sort, ctx.dist_arr_buff, sizeof(dist_arr_sort));
                ace_sorting::insertionSort(dist_arr_sort, DIST_ARRAY_LEN);

                ctx.dist = dist_arr_sort[DIST_ARRAY_LEN / 2];
                ctx.dist_valid = true;
            }
            else {
                ctx.dist_valid = false;
            }
        }

        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}