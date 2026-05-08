#pragma once

#include "../../lib/pt/pt.h"
#include "../../lib/GyverIO/GyverIO.h"
#include "../node/node_com.h"

struct ComController {
    struct pt pt_task;

    ComNode node;
    uint16_t last_seq = node.seq;

    void init() {
        
        node.port = &Serial2;
        PT_INIT(&pt_task);
    }


    int update(robot::TaskRobot *task) {
        node_com_run(node);

        PT_BEGIN(&pt_task);
        for (;;) {

            PT_WAIT_WHILE(&pt_task, last_seq != node.seq);
            last_seq = node.seq;

/*            if (node.command == PD_COMMAND_PING) {
                node_com_response(node, PD_RESPONSE_PING);
            }
            else if (node.command == PD_COMMAND_STEP) {
                if (node.mode == PD_COMMAND_STEP_UP)
                    *task = robot::TaskRobot_STEP_UP;
                else if (node.mode == PD_COMMAND_PACK_L2)
                    *task = robot::TaskRobot_VICTIM_LEFT_X2;
                else if (node.mode == PD_COMMAND_PACK_R1)
                    *task = robot::TaskRobot_VICTIM_RIGHT;
                else if (node.mode == PD_COMMAND_PACK_R2)
                    *task = robot::TaskRobot_VICTIM_RIGHT_X2;
            }
            else if (node.command == PD_COMMAND_PACK) {
                if (node.mode == PD_COMMAND_PACK_L1)
                    *task = robot::TaskRobot_VICTIM_LEFT;
                else if (node.mode == PD_COMMAND_PACK_L2)
                    *task = robot::TaskRobot_VICTIM_LEFT_X2;
                else if (node.mode == PD_COMMAND_PACK_R1)
                    *task = robot::TaskRobot_VICTIM_RIGHT;
                else if (node.mode == PD_COMMAND_PACK_R2)
                    *task = robot::TaskRobot_VICTIM_RIGHT_X2;
            }*/
        }
        PT_END(&pt_task);
    }
};
