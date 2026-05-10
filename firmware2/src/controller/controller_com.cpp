#include "controller_com.h"
#include "../robot/robot.h"

void ComController::init() {
    node.port = &Serial2;
}

int ComController::update(robot::TaskRobot* task) {
    node_com_run(node);

    if (node.command != 0 && node.command != ' ') {
        if (node.command == 'u') *task = robot::TaskRobot_STEP_UP;
        else if (node.command == 'l') *task = robot::TaskRobot_STEP_LEFT;
        else if (node.command == 'r') *task = robot::TaskRobot_STEP_RIGHT;
        else if (node.command == 'd') *task = robot::TaskRobot_STEP_DOWN;
        
        else if (node.command == 'v') *task = robot::TaskRobot_STEP_RIGHT;
        else if (node.command == 'b') *task = robot::TaskRobot_VICTIM_LEFT;
        else if (node.command == 'n') *task = robot::TaskRobot_VICTIM_RIGHT_X2;
        else if (node.command == 'm') *task = robot::TaskRobot_VICTIM_LEFT_X2;



        node.command = 0;
    }
}

#define MAX_DIST 190
#define MAX_DIST_UP 100

void ComController::sentData(uint16_t wallsMaze[4], uint16_t wallsRobot[4], 
    bool black, bool graw, uint8_t step_count)
{
    node.request[0] = (wallsMaze[0] < MAX_DIST_UP && wallsMaze[0] != 0) ? '1' : '0';
    node.request[1] = (wallsMaze[1] < MAX_DIST    && wallsMaze[1] != 0) ? '1' : '0';
    node.request[2] = (wallsMaze[2] < MAX_DIST    && wallsMaze[2] != 0) ? '1' : '0';
    node.request[3] = (wallsMaze[3] < MAX_DIST    && wallsMaze[3] != 0) ? '1' : '0';

    node.request[4] = (wallsRobot[0] < MAX_DIST_UP && wallsRobot[0] != 0) ? '1' : '0';
    node.request[5] = (wallsRobot[1] < MAX_DIST    && wallsRobot[1] != 0) ? '1' : '0';
    node.request[6] = (wallsRobot[2] < MAX_DIST    && wallsRobot[2] != 0) ? '1' : '0';
    node.request[7] = (wallsRobot[3] < MAX_DIST    && wallsRobot[3] != 0) ? '1' : '0';

    node.request[8] = black ? '1' : '0';
    node.request[9] = graw ? '1' : '0';
    
    char step_char;
    if (step_count == 0) step_char = '0';
    else if (step_count == 1) step_char = '1'
    else if (step_count == 2) step_char = '2';
}