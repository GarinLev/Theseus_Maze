#include "robot.h"
#include <Wire.h>
#include "../controller/controller_com.h"

namespace robot {
    TaskRobot task = TaskRobot_WAIT;
    StateRobot state = StateRobot_WAIT;

    WallManager wallManager;
    WallController wallRight, wallLeft;
    WheelManager wheelManager;
    WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    DebugController debug;
    RotateController rotateController;
    ServoController servoController;
    LightController lightController;
    struct pt pt_state;

#ifdef COM_ENABLE
    ComController comController;
#endif // COM_ENABLE


    static void sentData();
    static void waitForSensors(uint32_t timeout_ms);

    void init() {
        Serial.begin(115200);
        Serial2.begin(9600);

        Wire.begin();
        Wire.setClock(400000);

        wheelA1.init(500, 140); wheelA1.setPins(4, 5, false, 2, 22, robot::encoder::isr_A1);
        wheelA2.init(500, 140); wheelA2.setPins(8, 9, false, 18, 24, robot::encoder::isr_A2);
        wheelB1.init(500, 140); wheelB1.setPins(6, 7, true, 3, 23, robot::encoder::isr_B1);
        wheelB2.init(500, 140); wheelB2.setPins(10, 12, true, 19, 25, robot::encoder::isr_B2);

        wheelManager.fr = &wheelA1;
        wheelManager.fl = &wheelA2;
        wheelManager.br = &wheelB1;
        wheelManager.bl = &wheelB2;
        wheelManager.init();
        wheelManager.setDistSource(&wallRight.nodeExtra);
        wheelManager.setPitchSource(&rotateController.angel.ypr[1]);

        wallRight.setPins(&Wire, 0x30, 0x31, 0x32, 37, 36, 32);
        wallLeft.setPins(&Wire, 0x33, 0x34, 0x35, 33, 34, 35);

        wallRight.reset();
        wallLeft.reset();

        delay(50);

        wallRight.setAddr();
        wallLeft.setAddr();
        

        wallRight.init();
        wallLeft.init();


        wallManager.wheelA1 = &wheelA1;
        wallManager.wheelA2 = &wheelA2;
        wallManager.wheelB1 = &wheelB1;
        wallManager.wheelB2 = &wheelB2;
        wallManager.sens_left = &wallLeft;
        wallManager.sens_right = &wallRight;
        wallManager.init();
        Serial.print(3);

        rotateController.wheelA1 = &wheelA1;
        rotateController.wheelA2 = &wheelA2;
        rotateController.wheelB1 = &wheelB1;
        rotateController.wheelB2 = &wheelB2;
        rotateController.init();
        Serial.print(2);

        servoController.pin = 44;
        servoController.init();

        lightController.init();
        PT_INIT(&pt_state);

#ifdef COM_ENABLE
        comController.init();
        Serial.print(1);
        for (;;) {

            comController.node.request[0] = 0;
            node_com_run(comController.node);
            if (comController.node.command == 's') break;
            comController.node.request[0] = 's';
            node_com_run(comController.node);
            delay(100);
        }
        waitForSensors(800);
        sentData();
#endif // COM_ENABLE


        delay(500);
    }

    void loop() {
        debug.frame();
        debug.update();
        debug.ups_update();

#ifdef COM_ENABLE
        comController.update(&task);
#endif

        state_update();

        node_dist_run(wallRight.nodeWall);
        node_dist_run(wallLeft.nodeWall);
        node_dist_run(wallRight.nodeExtra);
        node_dist_run(wallLeft.nodeExtra);


        if (state == StateRobot_WAIT) handleWait();
        else if (state == StateRobot_MOVE) handleMove();
        else if (state == StateRobot_ROTATE) handleRotate();
        else if (state == StateRobot_VICTIM) handleVictim();

        lightController.update();
        wheelManager.update();
    }

    static void waitForSensors(uint32_t timeout_ms) {
#ifdef COM_ENABLE
        uint32_t start = millis();
        while (millis() - start < timeout_ms) {
            node_dist_run(wallRight.nodeUp);
            node_dist_run(wallRight.nodeWall);
            node_dist_run(wallRight.nodeExtra);
            node_dist_run(wallLeft.nodeUp);
            node_dist_run(wallLeft.nodeWall);
            node_dist_run(wallLeft.nodeExtra);

            delay(5);
        }
#endif // COM_ENABLE
    }

    static void sentData() {
#ifdef COM_ENABLE

        node_dist_run(wallRight.nodeWall);
        node_dist_run(wallLeft.nodeWall);
        node_dist_run(wallRight.nodeExtra);
        node_dist_run(wallLeft.nodeExtra);

        uint16_t distUp    = wallRight.nodeExtra.dist_valid ? wallRight.nodeExtra.dist : (uint16_t)0;
        uint16_t distLeft  = wallLeft.nodeWall.dist_valid ? wallLeft.nodeWall.dist : (uint16_t)0;
        uint16_t distDown  = wallLeft.nodeExtra.dist_valid ? wallLeft.nodeExtra.dist : (uint16_t)0;
        uint16_t distRight = wallRight.nodeWall.dist_valid ? wallRight.nodeWall.dist : (uint16_t)0;
        uint16_t wallsRobot[4] = { distUp, distLeft, distDown, distRight };

        uint16_t mazeUp, mazeLeft, mazeDown, mazeRight;

        if (rotateController.rotate_now == RotateController::UP) {
            mazeUp = distUp;    mazeRight = distRight; mazeDown = distDown;  mazeLeft = distLeft;
        }
        else if (rotateController.rotate_now == RotateController::RIGHT) {
            mazeUp = distLeft;  mazeRight = distUp;    mazeDown = distRight; mazeLeft = distDown;
        }
        else if (rotateController.rotate_now == RotateController::DOWN) {
            mazeUp = distDown;  mazeRight = distLeft;  mazeDown = distUp;    mazeLeft = distRight;
        }
        else if (rotateController.rotate_now == RotateController::LEFT) {
            mazeUp = distRight; mazeRight = distDown; mazeDown = distLeft;  mazeLeft = distUp;
        }


        uint16_t walls[4] = { mazeUp, mazeLeft, mazeDown, mazeRight };

        uint8_t step_count = 1;
        if (wheelManager.hasDouble) {
            wheelManager.hasDouble = false;
            step_count = 2;
        } 

        Serial.print(mazeUp); Serial.print("\t");
        Serial.print(mazeLeft); Serial.print("\t");
        Serial.print(mazeDown); Serial.print("\t");
        Serial.println(mazeRight);

        comController.sentData(walls, wallsRobot, false, false, step_count);
        comController.update(&task);

#endif // COM_ENABLE
    } 

    int state_update() {
        PT_BEGIN(&pt_state);

        PT_WAIT_UNTIL(&pt_state, task != TaskRobot_WAIT);

        if (task == TaskRobot_STEP_UP || task == TaskRobot_STEP_DOWN ||
            task == TaskRobot_STEP_LEFT || task == TaskRobot_STEP_RIGHT) {

            state = StateRobot_ROTATE;
            if (task == TaskRobot_STEP_UP) rotateController.run(RotateController::UP, 100);
            else if (task == TaskRobot_STEP_DOWN) rotateController.run(RotateController::DOWN, 100);
            else if (task == TaskRobot_STEP_RIGHT) rotateController.run(RotateController::LEFT, 100);
            else rotateController.run(RotateController::RIGHT, 100);

            PT_WAIT_UNTIL(&pt_state, !rotateController.is_active);

            state = StateRobot_WAIT;
            { uint32_t _tmr = millis();
            PT_WAIT_UNTIL(&pt_state, millis() - _tmr >= 500); }

            state = StateRobot_MOVE;
            wheelManager.moveDistance(300.0f, 100.0f, nullptr);
            PT_WAIT_UNTIL(&pt_state, !wheelManager.is_moving);

            state = StateRobot_WAIT;
            { uint32_t _tmr = millis();
            PT_WAIT_UNTIL(&pt_state, millis() - _tmr >= 500); }

            task = TaskRobot_WAIT;
            waitForSensors(1000);
            sentData();
        }
        else if (task == TaskRobot_VICTIM_RIGHT || task == TaskRobot_VICTIM_RIGHT_X2 ||
                 task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2) {

            state = StateRobot_VICTIM;

            servoController.set((task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2) ? 1 : 2);
            PT_WAIT_UNTIL(&pt_state, !servoController.is_active);

            servoController.set((task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2) ? 3 : 4);
            PT_WAIT_UNTIL(&pt_state, !servoController.is_active);

            if (task == TaskRobot_VICTIM_LEFT_X2 || task == TaskRobot_VICTIM_RIGHT_X2) {
                servoController.set((task == TaskRobot_VICTIM_LEFT_X2) ? 1 : 2);
                PT_WAIT_UNTIL(&pt_state, !servoController.is_active);

                servoController.set((task == TaskRobot_VICTIM_LEFT_X2) ? 3 : 4);
                PT_WAIT_UNTIL(&pt_state, !servoController.is_active);
            }

            Serial.println("END VICTIM");
            task = TaskRobot_WAIT;
        }

        PT_END(&pt_state);
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