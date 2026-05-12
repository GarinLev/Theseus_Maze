#pragma once
#include "../../lib/pt/pt.h"
#include "../node/node_com.h"
#include "../robot/robot.h"

struct ComController {
    struct pt pt_task;
    ComNode node;

    void init();
    void update(robot::TaskRobot* task);
    void ComController::sentData(uint16_t walls[4], bool graw, uint8_t step_count);
};