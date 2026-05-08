#pragma once
#include "../../lib/pt/pt.h"
#include "../node/node_com.h"
#include "../robot/robot.h"

struct ComController {
    struct pt pt_task;
    ComNode node;
    uint16_t last_seq;

    void init();
    int update(robot::TaskRobot* task);
};