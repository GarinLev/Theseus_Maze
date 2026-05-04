#pragma once

#include "../macro.h"

struct EncoderTopic {
    TOPIC_FIELDS;
    volatile int32_t value = 0;
    volatile int32_t period = 0;
    volatile bool reverse = false;
};