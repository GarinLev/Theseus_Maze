#pragma once

#include "../../macro.h"

struct MotorTopic {
    TOPIC_FIELDS;
    int16_t speed = 0;
};