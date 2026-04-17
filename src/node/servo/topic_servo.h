#pragma once
#include "../../macro.h"

struct ServoTopic {
    TOPIC_FIELDS;
    float value;
    uint32_t time;
    float d_acc;
};