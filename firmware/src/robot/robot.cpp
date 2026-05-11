#include "robot.h"
#include <Wire.h>
#include "../controller/controller_com.h"

namespace robot {
    struct pt task_step;
    struct pt task_victim;

    TaskRobot task = TaskRobot_WAIT;
    TaskRobot saved_task = TaskRobot_WAIT;
    StateRobot state = StateRobot_WAIT;
    SubTask current_sub_step = SUB_START;

    WallManager wallManager;
    WallController wallRight, wallLeft;
    WheelManager wheelManager;
    WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    DebugController debug;
    RotateController rotateController;
    ServoController servoController;
    LightController lightController;

#ifdef COM_ENABLE
    ComController comController;
#endif 

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

        wheelManager.fr = &wheelA1; wheelManager.fl = &wheelA2;
        wheelManager.br = &wheelB1; wheelManager.bl = &wheelB2;
        wheelManager.init();
        wheelManager.setDistSource(&wallRight.nodeExtra);
        wheelManager.setPitchSource(&rotateController.angel.ypr[1]);

        wallRight.setPins(&Wire, 0x30, 0x31, 0x32, 37, 36, 32);
        wallLeft.setPins(&Wire, 0x33, 0x34, 0x35, 33, 34, 35);
        wallRight.reset(); wallLeft.reset();
        delay(50);
        wallRight.setAddr(); wallLeft.setAddr();
        wallRight.init(); wallLeft.init();

        rotateController.wheelA1 = &wheelA1; rotateController.wheelA2 = &wheelA2;
        rotateController.wheelB1 = &wheelB1; rotateController.wheelB2 = &wheelB2;
        rotateController.init();

        servoController.pin = 44;
        servoController.init();
        lightController.init();
        lightController.on();

        // PT_INIT(&task_step);
        // PT_INIT(&task_victim);

#ifdef COM_ENABLE
        comController.init();
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
#endif
        delay(500);
    }

    void loop() {
        debug.frame();
        debug.update();
        debug.ups_update();

#ifdef COM_ENABLE
        comController.update(&task);
#endif

        node_dist_run(wallRight.nodeWall);
        node_dist_run(wallLeft.nodeWall);
        node_dist_run(wallRight.nodeExtra);
        node_dist_run(wallLeft.nodeExtra);

        if (task >= TaskRobot_VICTIM_LEFT && task <= TaskRobot_VICTIM_RIGHT_X2) {
            update_victim();
        }
        else if (task >= TaskRobot_STEP_UP && task <= TaskRobot_STEP_DOWN) {
            update_step();
        }
        else {
            state = StateRobot_WAIT;
        }

        if (state == StateRobot_WAIT) handleWait();
        else if (state == StateRobot_MOVE) handleMove();
        else if (state == StateRobot_ROTATE) handleRotate();
        else if (state == StateRobot_VICTIM) handleVictim();

        lightController.update();
        wheelManager.update();
    }


    int update_step() {
        PT_BEGIN(&task_step);
        static uint32_t timer;

        if (current_sub_step == SUB_START) {
            Serial.println("SUB_START - S");
            state = StateRobot_ROTATE;
            if (task == TaskRobot_STEP_DOWN) rotateController.run(175, 100);
            else if (task == TaskRobot_STEP_RIGHT) rotateController.run(85, 100);
            else if (task == TaskRobot_STEP_LEFT) rotateController.run(-85, 100);

            current_sub_step = SUB_ROTATING;

            Serial.println("SUB_START - E");
        }

        if (current_sub_step == SUB_ROTATING) {
            Serial.println("SUB_ROTATE - S");
            if (task != TaskRobot_STEP_UP) {
                PT_WAIT_WHILE(&task_step, rotateController.is_active);
                timer = millis();
                PT_WAIT_WHILE(&task_step, millis() - timer < 200);
            }
            current_sub_step = SUB_MOVING;
            Serial.println("SUB_ROTATE - E");
        }

        if (current_sub_step == SUB_MOVING) {
            Serial.println("SUB_MOVING - S");
            state = StateRobot_MOVE;
            wheelManager.moveDistance(300.0f, 100.0f, nullptr);
            PT_WAIT_WHILE(&task_step, wheelManager.is_moving);

            timer = millis();
            PT_WAIT_WHILE(&task_step, millis() - timer < 200);
            current_sub_step = SUB_DONE;
            Serial.println("SUB_MOVING - E");
        }

        Serial.println("SUB_SEND");
        sentData();
        task = TaskRobot_WAIT;
        current_sub_step = SUB_START;

        Serial.println("SUB_END");
        PT_END(&task_step);
    }

    int update_victim() {
        PT_BEGIN(&task_victim);
        static bool isLeft, isX2;

        state = StateRobot_VICTIM;
        isLeft = (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2);
        isX2 = (task == TaskRobot_VICTIM_LEFT_X2 || task == TaskRobot_VICTIM_RIGHT_X2);

        servoController.set(isLeft ? ServoController_Left : ServoController_Right);
        PT_WAIT_WHILE(&task_victim, servoController.is_active);
        servoController.set(isLeft ? ServoController_CloseLeft : ServoController_CloseRight);
        PT_WAIT_WHILE(&task_victim, servoController.is_active);

        if (isX2) {
            servoController.set(isLeft ? ServoController_Left : ServoController_Right);
            PT_WAIT_WHILE(&task_victim, servoController.is_active);
            servoController.set(isLeft ? ServoController_CloseLeft : ServoController_CloseRight);
            PT_WAIT_WHILE(&task_victim, servoController.is_active);
        }

        if (robot::saved_task != TaskRobot_WAIT) {
            task = robot::saved_task;
            robot::saved_task = TaskRobot_WAIT;
            PT_INIT(&task_step);
        }
        else {
            task = TaskRobot_WAIT;
            current_sub_step = SUB_START;
            sentData();
        }

        PT_END(&task_victim);
    }

 
    static void waitForSensors(uint32_t timeout_ms) {
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
    }

    static void sentData() {
#ifdef COM_ENABLE
        uint16_t distUp = wallRight.nodeExtra.dist_valid ? wallRight.nodeExtra.dist : 0;
        uint16_t distLeft = wallLeft.nodeWall.dist_valid ? wallLeft.nodeWall.dist : 0;
        uint16_t distDown = wallLeft.nodeExtra.dist_valid ? wallLeft.nodeExtra.dist : 0;
        uint16_t distRight = wallRight.nodeWall.dist_valid ? wallRight.nodeWall.dist : 0;

        uint16_t walls[4] = { distUp, distLeft, distDown, distRight };
        uint8_t step_count = wheelManager.hasDouble ? 2 : 1;
        wheelManager.hasDouble = false;

        comController.sentData(walls, false, false, step_count);
#endif
    }

    void handleWait() { wheelManager.stop(); }
    void handleRotate() { rotateController.update(); }
    void handleVictim() { servoController.update(); }
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
}