#pragma once
#include "../../lib/pt/pt.h"
#include "protocol_defs.h"

struct ComNode {
    struct pt pt;
    Stream* port;

    uint8_t rx_buff[PD_COMMAND_LEN] = {};
    uint8_t tx_buff[PD_COMMAND_LEN] = {};
    bool tx_available = false;
    uint16_t seq = 0;

    uint8_t command = 0;
    uint8_t mode = 0;
};

void node_com_init(ComNode& ctx, Stream* port);
int node_com_run(ComNode& ctx);
void node_com_response(ComNode& ctx, uint8_t status);