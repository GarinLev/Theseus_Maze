#pragma once

#include "../macro.h"

struct EncoderTopic {
    TOPIC_FIELDS;
    volatile int32_t value = 0;
    volatile uint32_t period = 0;
};