#pragma once
#include <Servo.h>
#include "topic_servo.h"
#include "../../../lib/pt/pt.h"
#include "../../macro.h"
#include "../../soft_move.h"

struct ServoNode {
    struct pt pt;
    SUB_FIELDS;
    ServoTopic* topic;

    int pin;
    Servo driver;

    SoftMove move;
    uint32_t startTime;
    uint32_t last_process_time;
    float currentPos;

    uint32_t dt;
};

void node_servo_init(ServoNode& ctx);
int  node_servo_run(ServoNode& ctx);