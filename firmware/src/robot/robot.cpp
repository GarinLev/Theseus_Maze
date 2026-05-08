#include "robot.h"
#include <Wire.h>
#include "../node/node_angel.h"
#include "../controller/controller_com.h"

namespace robot {
    TaskRobot task = TaskRobot_WAIT;
    StateRobot state = StateRobot_WAIT;
    struct pt thread;

    StateFunc stateTable[] = { handleWait, handleMove, handleRotate, handleVictim };

    // Определение объектов
    WallManager wallManager;
    WallController wallRight, wallLeft;
    WheelManager wheelManager;
    WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    DebugController debug;
    RotateController rotateController;
    ServoController servoController;
    ComController comController;


    void init() {
        Serial.begin(115200);
        Serial2.begin(9600);
        Wire.begin();
        Wire.setClock(400000);

        wheelA1.init(500, 140); wheelA1.setPins(4, 5, false, 2, 22, isr_encoder_A1);
        wheelA2.init(500, 140); wheelA2.setPins(8, 9, false, 18, 24, isr_encoder_A2);
        wheelB1.init(500, 140); wheelB1.setPins(6, 7, true, 3, 23, isr_encoder_B1);
        wheelB2.init(500, 140); wheelB2.setPins(10, 12, true, 19, 25, isr_encoder_B2);

        wheelManager.fr = &wheelA1; wheelManager.fl = &wheelA2;
        wheelManager.br = &wheelB1; wheelManager.bl = &wheelB2;
        wheelManager.init();

        rotateController.wheelA1 = &wheelA1; rotateController.wheelA2 = &wheelA2;
        rotateController.wheelB1 = &wheelB1; rotateController.wheelB2 = &wheelB2;
        rotateController.init();

        servoController.pin = 44;
        servoController.init();

        comController.init();
        for (;;) {
            // node_com_response(comController.node, PD_USE_START);
            // comController.update(&task);
            if (comController.node.command == PD_COMMAND_PING)
            {
                break;
            }

            delay(200);
        }
        Serial.println("Sucsesfull!");
        

        PT_INIT(&thread);
    }

    void loop() {
        debug.frame();
        debug.update();
        debug.ups_update();

        comController.update(&task);

        state_update();

        if (stateTable[state]) {
            stateTable[state]();
        }

        wheelManager.update();
    }

    int state_update() {
        PT_BEGIN(&thread);
        for (;;) {
            if (task == TaskRobot_STEP_UP) {
                state = StateRobot_MOVE;
                wheelManager.moveDistance(300.0f, 100.0f, &rotateController.angel.ypr[0]);
                PT_WAIT_WHILE(&thread, wheelManager.is_moving);
                task = TaskRobot_WAIT;
            }
            else if (task >= TaskRobot_VICTIM_LEFT && task <= TaskRobot_VICTIM_RIGHT_X2) {
                state = StateRobot_VICTIM;
                bool isLeft = (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2);
                bool isX2 = (task == TaskRobot_VICTIM_LEFT_X2 || task == TaskRobot_VICTIM_RIGHT_X2);

                servoController.set(isLeft ? 1 : 2);
                PT_WAIT_WHILE(&thread, servoController.is_active);

                servoController.set(isLeft ? 3 : 4);
                PT_WAIT_WHILE(&thread, servoController.is_active);

                if (isX2) {
                    servoController.set(isLeft ? 1 : 2);
                    PT_WAIT_WHILE(&thread, servoController.is_active);
                    servoController.set(isLeft ? 3 : 4);
                    PT_WAIT_WHILE(&thread, servoController.is_active);
                }
                task = TaskRobot_WAIT;
            }
            else if (task == TaskRobot_WAIT) {
                state = StateRobot_WAIT;
            }
            PT_YIELD(&thread);
        }
        PT_END(&thread);
    }

    void handleWait() {
        wheelManager.stop();
    }

    void handleMove() {
        node_angel_run(rotateController.angel);
        if (!wheelManager.wall_disable) {
            wallRight.update();
            wallLeft.update();
            wallManager.update();
        }
        else {
            wheelA1.speed_offset = wheelA2.speed_offset = 0;
            wheelB1.speed_offset = wheelB2.speed_offset = 0;
        }
    }

    void handleRotate() {
        rotateController.update();
    }

    void handleVictim() {
        servoController.update();
    }

} // namespace robot