#pragma once

#include "../controller/controller_wheel.h"
#include "../controller/controller_wall.h"
#include "../controller/manager_wall.h"
#include "../controller/manager_wheel.h"
#include "../controller/controller_debug.h"
#include "../controller/controller_rotate.h"

namespace robot {
    extern pt task;
    enum StateRobot {
        WAIT = 0,
        MOVE = 1,
        ROTATE = 2
    };
    extern StateRobot stateRobot;
    typedef void (*StateFunc)();
    extern StateFunc stateTable[];
    

    extern WallManager wallManager;
    extern WallController wallRight;
    extern WallController wallLeft;

    extern WheelManager wheelManager;
    extern WheelController wheelA1;
    extern WheelController wheelA2;
    extern WheelController wheelB1;
    extern WheelController wheelB2;

    extern DebugController debug;
    extern RotateController rotateManager;

    int state_update();
    void init();
    void loop();

    void handleWait();
    void handleMove();
    void handleRotate();

    void step();
    void rotate(Rotates rotate);

    void isr_encoder_A1();
    void isr_encoder_A2();
    void isr_encoder_B1();
    void isr_encoder_B2();
}
