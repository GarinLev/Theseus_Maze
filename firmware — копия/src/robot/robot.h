#pragma once

#include "../controller/controller_wheel.h"
#include "../controller/controller_wall.h"
#include "../controller/manager_wall.h"
#include "../controller/manager_wheel.h"
#include "../controller/controller_debug.h"
#include "../controller/controller_rotate.h"
#include "../controller/controller_hit.h"
#include "../controller/controller_servo.h"

namespace robot {
    extern pt thread;
    enum StateRobot {
        StateRobot_WAIT = 0,
        StateRobot_MOVE = 1,
        StateRobot_ROTATE = 2,
        StateRobot_VICTIM = 3
    };
    enum TaskRobot {
        TaskRobot_WAIT,
        TaskRobot_STEP_UP,
        TaskRobot_STEP_DOWN,
        TaskRobot_STEP_LEFT,
        TaskRobot_STEP_RIGHT,
        TaskRobot_VICTIM_LEFT,
        TaskRobot_VICTIM_RIGHT,
        TaskRobot_VICTIM_LEFT_X2,
        TaskRobot_VICTIM_RIGHT_X2
    };

    extern TaskRobot task;
    extern StateRobot state;
    typedef void (*StateFunc)();
    extern StateFunc stateTable[];
    
    // extern ComController comController;

    extern WallManager wallManager;
    extern WallController wallRight;
    extern WallController wallLeft;
    extern HitController hitController;

    extern WheelManager wheelManager;
    extern WheelController wheelA1;
    extern WheelController wheelA2;
    extern WheelController wheelB1;
    extern WheelController wheelB2;

    extern DebugController debug;
    extern RotateController rotateManager;
    extern ServoController servoController;

    int state_update();
    void init();
    void loop();

    void handleWait();
    void handleMove();
    void handleRotate();
    void handleVictim();

    void isr_encoder_A1();
    void isr_encoder_A2();
    void isr_encoder_B1();
    void isr_encoder_B2();
}
