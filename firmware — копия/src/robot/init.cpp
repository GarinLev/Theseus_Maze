#include "robot.h"

namespace robot {
    struct pt thread;
    StateRobot state = StateRobot_WAIT;
    StateFunc stateTable[] = {
            &robot::handleWait,
            &robot::handleMove,
            &robot::handleRotate,
            &robot::handleVictim
    };
    TaskRobot task = TaskRobot_WAIT;

    // ComController comController;

    WallManager wallManager;
    WallController wallRight;
    WallController wallLeft;
    HitController hitController;

    WheelManager wheelManager;
    WheelController wheelA1;
    WheelController wheelA2;
    WheelController wheelB1;
    WheelController wheelB2;

    DebugController debug;
    AngelNode mpu;
    RotateController rotateManager;
    ServoController servoController;

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
    }

    static void initRotate() {
        rotateManager.init();

        rotateManager.wheelA1 = &wheelA1;
        rotateManager.wheelA2 = &wheelA2;
        rotateManager.wheelB1 = &wheelB1;
        rotateManager.wheelB2 = &wheelB2;
    }

    static void initHit() {
        hitController.pinA = 40;
        hitController.pinB = 41;
        hitController.wheelManager = &wheelManager;
        hitController.sens = &wallRight.nodeExtra;

        hitController.init();
    }

    static void initServo() {
        servoController.pin = 44;
        servoController.init();
    }


    void init() {
        Serial.begin(115200);
        Wire.begin();
        Wire.setClock(400000);

        initWheel();
        initWall();
        initRotate();
        initHit();
        initServo();

        PT_INIT(&thread);
    }
}

 