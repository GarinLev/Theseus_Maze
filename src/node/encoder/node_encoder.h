#pragma once


#include "../../../lib/pt/pt.h"
#include "../../macro.h"

#include "topic_encoder.h"

struct EncoderNode {
    struct pt pt;
    SUB_FIELDS;

    EncoderTopic *topic;
    uint64_t timer = 0;
    
    void (*func)(void);
    
    uint8_t pin_int;
    uint8_t pin_dir;
};

void node_encoder_init(EncoderNode &ctx);