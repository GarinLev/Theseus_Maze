#include "robot.h"

namespace robot {
    pt task;
    StateRobot stateRobot;
    StateFunc stateTable[] = {
            &robot::handleWait,
            &robot::handleMove,
            &robot::handleRotate
    };

    WallManager wallManager;
    WallController wallRight;
    WallController wallLeft;

    WheelManager wheelManager;
    WheelController wheelA1;
    WheelController wheelA2;
    WheelController wheelB1;
    WheelController wheelB2;

    DebugController debug;
    AngelNode mpu;
    RotateController rotateManager;

    static void initWheel() {
        wheelA1.init(500.0f, 140.0f);
        wheelA1.setPins(4, 5, false, 2, 22, robot::isr_encoder_A1);

        wheelA2.init(500.0f, 140.0f);
        wheelA2.setPins(8, 9, false, 18, 24, robot::isr_encoder_A2);

        wheelB1.init(500.0f, 140.0f);
        wheelB1.setPins(6, 7, true, 3, 23, robot::isr_encoder_B1);

        wheelB2.init(500.0f, 140.0f);
        wheelB2.setPins(10, 12, true, 19, 25, robot::isr_encoder_B2);

        wheelManager.fr = &wheelA1;
        wheelManager.fl = &wheelA2;
        wheelManager.br = &wheelB1;
        wheelManager.bl = &wheelB2;
    }

    static void initWall() {
        pinMode(37, OUTPUT);
        pinMode(36, OUTPUT);
        pinMode(33, OUTPUT);
        pinMode(34, OUTPUT);

        wallRight.setPins(&Wire, 0x30, 0x31, 37, 36);
        wallLeft.setPins(&Wire, 0x32, 0x33, 33, 34);

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
    }

    void initRotate() {
        rotateManager.init();

        rotateManager.wheelA1 = &wheelA1;
        rotateManager.wheelA2 = &wheelA2;
        rotateManager.wheelB1 = &wheelB1;
        rotateManager.wheelB2 = &wheelB2;
    }

    void init() {
        Serial.begin(115200);
        Wire.begin();
        // Wire.setClock(400000);

        initWheel();
        initWall();
        initRotate();

        PT_INIT(&task);
    }
}
