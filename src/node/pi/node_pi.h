#pragma once

#include "../../../lib/pt/pt.h"
#include "../../macro.h"
#include "topic_pi.h"

struct PI_Node {
    struct pt pt;
    SUB_FIELDS;

    uint32_t dt;
    uint32_t last_time;

    int16_t setpoint;
    float integral = 0;

    float Kp, Ki;

    float value_out;
    PI_Topic* topic;
};

void node_pi_init(PI_Node& ctx);
int node_pi_run(PI_Node& ctx);
