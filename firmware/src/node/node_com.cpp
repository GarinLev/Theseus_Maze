#include "node_com.h"

static uint8_t calc_crc8(uint8_t* data, int len) {
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (uint8_t)(crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

void node_com_init(ComNode& ctx, Stream* port) {
    PT_INIT(&ctx.pt);
    ctx.port = port;
}


int node_com_run(ComNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {

        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}