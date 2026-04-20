#pragma once

#include "../../../lib/pt/pt.h"
#include "../../soft_move.h"

struct MotorTargetNode {
    struct pt pt;
    SoftMove soft;
    uint32_t last_time;
};

void node_motorT_init(MotorTargetNode& ctx);
float node_motorT_step(MotorTargetNode& ctx, uint32_t x);
