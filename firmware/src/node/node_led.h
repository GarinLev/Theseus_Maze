#pragma once

#include "topic_led.h"

#include "../../lib/pt/pt.h"
#include "../macro.h"

struct LedNode {
    struct pt pt;
    SUB_FIELDS;
    LedTopic *topic;
    
    int pin;
};

void node_led_init(LedNode &ctx);
int  node_led_run(LedNode &ctx); 