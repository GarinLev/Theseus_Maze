#include "node_com.h"
#include "protocol_defs.h"

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

static void tx_update(ComNode& ctx) {
    if (ctx.tx_available) {
        ctx.port->write(ctx.tx_buff, PD_COMMAND_LEN);
        ctx.tx_available = false;
    }
}

void node_com_response(ComNode& ctx, uint8_t status) {
    ctx.tx_buff[0] = PD_START;
    ctx.tx_buff[1] = status;

    for (int i = 2; i < PD_COMMAND_LEN - 1; i++) {
        ctx.tx_buff[i] = 0;
    }
    
    ctx.tx_buff[PD_COMMAND_LEN - 1] = calc_crc8(ctx.tx_buff, PD_COMMAND_LEN - 1);
    ctx.tx_available = true;
}

int node_com_run(ComNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        tx_update(ctx);

        PT_WAIT_UNTIL(&ctx.pt, ctx.port->available() > 0);

        if (ctx.port->read() == PD_START) {
            ctx.rx_buff[0] = PD_START;

            for (static int i = 1; i < PD_COMMAND_LEN; i++) {
                PT_WAIT_UNTIL(&ctx.pt, ctx.port->available() > 0);
                ctx.rx_buff[i] = ctx.port->read();
            }

            if (calc_crc8(ctx.rx_buff, PD_COMMAND_LEN - 1) != ctx.rx_buff[PD_COMMAND_LEN - 1]) {
                node_com_response(ctx, PD_RESPONSE_ERR_CRC8);
                continue;
            }

            uint8_t command = (ctx.rx_buff[1] >> 4) & 0x0F;
            uint8_t mode = ctx.rx_buff[1] & 0x0F;

            if (command == PD_COMMAND_PING) {
                node_com_response(ctx, PD_RESPONSE_PING);
            }
            else if (command == PD_COMMAND_STEP ||
                command == PD_COMMAND_PACK ||
                command == PD_USE) {

                ctx.command = command;
                ctx.mode = mode;

                node_com_response(ctx, PD_RESPONSE_OK);
                ctx.seq += 1;
            }
            else {
                node_com_response(ctx, PD_RESPONSE_ERR_COMMAND);
            }
        }

        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}
