#include "node_com.h"
#include "protocol_defs.h"


void node_com_init(ComNode& ctx, Stream* port) {
    PT_INIT(&ctx.pt);
    ctx.port = port;
}

static const uint8_t empty_request[10] = { 0 };
int node_com_run(ComNode& ctx) {

        if (ctx.port->available() > 0) {
            uint8_t data = ctx.port->read();
            Serial.print("HAS NEW DATA: "); Serial.println(char(data));
            ctx.command = data;
        }

        if (memcmp(ctx.request, empty_request, sizeof(ctx.request)) != 0) {
            if (ctx.request[0] != 's')
                Serial.print("SENT NEW DATA: ");


            for (uint8_t i = 0; i < 9; i++) {
                if (ctx.request[i] != 0)
                {
                    ctx.port->write(ctx.request[i]);
                    if (ctx.request[i] != 0 && ctx.request[0] != 's')
                        Serial.print(char(ctx.request[i]));
                    ctx.request[i] = 0;
                }
            }

            if (ctx.request[0] != 's')
                Serial.print("\n");

        }

}