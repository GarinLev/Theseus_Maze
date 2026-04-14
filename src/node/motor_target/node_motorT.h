#pragma once


#include "../../../lib/pt/pt.h"
#include "../../macro.h"
#include "../../soft_move.h"
#include "topic_motorT.h"

struct MotorTargetNode {
    struct pt pt;

    SoftMove soft;
    uint32_t last_time;

    MotorTopicTarget* topic_out;
};

void node_motorT_init(MotorTargetNode&ctx);
int node_motorT_run(MotorTargetNode& ctx, uint16_t x);