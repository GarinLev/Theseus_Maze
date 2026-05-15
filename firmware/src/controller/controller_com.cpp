#include "controller_com.h"
#include "../robot/robot.h"

#define MAX_DIST 230
#define MAX_DIST_UP 200

void ComController::init() {
    node.port = &Serial2;
}
void ComController::update(robot::TaskRobot* task) {
    node_com_run(node);

    if (node.command != 0 && node.command != ' ') {
        char cmd = node.command;
        node.command = 0;

        if (cmd == 'u' || cmd == 'l' || cmd == 'r' || cmd == 'd') {

            if (cmd == 'u')      *task = robot::TaskRobot_STEP_UP;
            else if (cmd == 'l') *task = robot::TaskRobot_STEP_LEFT;
            else if (cmd == 'r') *task = robot::TaskRobot_STEP_RIGHT;
            else if (cmd == 'd') *task = robot::TaskRobot_STEP_DOWN;
        }
        else if (cmd == 'v' || cmd == 'b' || cmd == 'n' || cmd == 'm') {
            if (cmd == 'v')      *task = robot::TaskRobot_VICTIM_RIGHT;
            else if (cmd == 'b') *task = robot::TaskRobot_VICTIM_LEFT;
            else if (cmd == 'n') *task = robot::TaskRobot_VICTIM_RIGHT_X2;
            else if (cmd == 'm') *task = robot::TaskRobot_VICTIM_LEFT_X2;
        }
    }
}

void ComController::sentData(uint16_t walls[4], bool graw, uint8_t step_count)
{
    char step_char;
    if (step_count == 0) step_char = '0';
    else if (step_count == 1) step_char = '1';
    else if (step_count == 2) step_char = '2';
    node.request[0] = (walls[0] < MAX_DIST_UP && walls[0] != 0) ? '1' : '0';
    node.request[1] = (walls[1] < MAX_DIST && walls[1] != 0) ? '1' : '0';
    node.request[2] = (walls[2] < MAX_DIST && walls[2] != 0) ? '1' : '0';
    node.request[3] = (walls[3] < MAX_DIST && walls[3] != 0) ? '1' : '0';
    node.request[4] = graw ? '1' : '0';
    node.request[5] = step_char;
}