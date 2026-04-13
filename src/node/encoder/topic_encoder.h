#pragma once

#include "../../macro.h"

struct EncoderTopic {
    TOPIC_FIELDS;
    volatile int16_t value = 0;
    volatile int16_t period = 0;
};