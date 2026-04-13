#pragma once


#include "../../../lib/pt/pt.h"
#include "../../macro.h"

#include "topic_motor.h"

struct MotorNode {
    struct pt pt;
    SUB_FIELDS;
    MotorTopic *topic;
    
    bool reverse;
    uint8_t pin_in1;
    uint8_t pin_in2;
};

void node_motor_init(MotorNode &ctx);
int  node_motor_run(MotorNode &ctx);