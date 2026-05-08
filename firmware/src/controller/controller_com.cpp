#include "controller_com.h"

void ComController::init() {
    node.port = &Serial2;
    PT_INIT(&pt_task);
    Serial.println("Init COM");
}


int ComController::update(robot::TaskRobot* task) {
    node_com_run(node);

    PT_BEGIN(&pt_task);
    for (;;) {
        
        PT_WAIT_WHILE(&pt_task, last_seq == node.seq);
        last_seq = node.seq;

        Serial.print(F("COM: Recv CMD=")); Serial.print(node.command);
        Serial.print(F(" MODE=")); Serial.println(node.mode);

        if (node.command == PD_COMMAND_PING) {
            Serial.println(F("COM: Ping -> Response"));
            node_com_response(node, PD_RESPONSE_PING);
        }
        else if (node.command == PD_COMMAND_STEP) {
            Serial.print(F("COM: Step Command -> "));
            if (node.mode == PD_COMMAND_STEP_UP) {
                *task = robot::TaskRobot_STEP_UP;
                Serial.println(F("STEP_UP"));
            }
            else if (node.mode == PD_COMMAND_PACK_L2) {
                *task = robot::TaskRobot_VICTIM_LEFT_X2;
                Serial.println(F("L_X2"));
            }
            else if (node.mode == PD_COMMAND_PACK_R1) {
                *task = robot::TaskRobot_VICTIM_RIGHT;
                Serial.println(F("R_1"));
            }
            else if (node.mode == PD_COMMAND_PACK_R2) {
                *task = robot::TaskRobot_VICTIM_RIGHT_X2;
                Serial.println(F("R_X2"));
            }
        }
        else if (node.command == PD_COMMAND_PACK) {
            Serial.print(F("COM: Pack Command -> "));
            if (node.mode == PD_COMMAND_PACK_L1) {
                *task = robot::TaskRobot_VICTIM_LEFT;
                Serial.println(F("L_1"));
            }
            else if (node.mode == PD_COMMAND_PACK_L2) {
                *task = robot::TaskRobot_VICTIM_LEFT_X2;
                Serial.println(F("L_X2"));
            }
            else if (node.mode == PD_COMMAND_PACK_R1) {
                *task = robot::TaskRobot_VICTIM_RIGHT;
                Serial.println(F("R_1"));
            }
            else if (node.mode == PD_COMMAND_PACK_R2) {
                *task = robot::TaskRobot_VICTIM_RIGHT_X2;
                Serial.println(F("R_X2"));
            }
        }
    }
    PT_END(&pt_task);
}