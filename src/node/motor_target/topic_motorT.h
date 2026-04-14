#pragma once

#include "../../macro.h"

struct MotorTopicTarget {
    TOPIC_FIELDS;

    int16_t speed = 0;
};