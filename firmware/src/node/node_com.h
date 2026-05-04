#pragma once
#include "../../lib/pt/pt.h"
#include "protocol_defs.h"

struct ComNode {
    struct pt pt;
    Stream* port;


};

void node_com_init(ComNode& ctx, Stream* port);
int node_com_run(ComNode& ctx);