#pragma once

#include "../../lib/pt/pt.h"
#include "protocol_defs.h"
#include <Arduino.h>

struct ComNode {
    struct pt pt;
    Stream* port;
    uint8_t command;
    uint8_t request[10];
    uint32_t seq;
};

void node_com_init(ComNode& ctx, Stream* port);
int node_com_run(ComNode& ctx);
