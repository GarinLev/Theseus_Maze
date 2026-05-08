#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include "../../lib/pt/pt.h"

#ifndef WC_MM_TO_TICKS
#define WC_MM_TO_TICKS(mm) ((mm) * 5.35f) 
#endif

#include "../controller/controller_wheel.h"
#include "../controller/controller_wall.h"
#include "../controller/manager_wall.h"
#include "../controller/manager_wheel.h"
#include "../controller/controller_rotate.h"
#include "../controller/controller_servo.h"
#include "../controller/controller_debug.h"

struct ComController;
namespace robot {
    enum TaskRobot {
        TaskRobot_WAIT,
        TaskRobot_STEP_UP,
        TaskRobot_VICTIM_LEFT,
        TaskRobot_VICTIM_RIGHT,
        TaskRobot_VICTIM_LEFT_X2,
        TaskRobot_VICTIM_RIGHT_X2,
        TaskRobot_TEST
    };
    enum StateRobot { StateRobot_WAIT, StateRobot_MOVE, StateRobot_ROTATE, StateRobot_VICTIM };
    typedef void (*StateFunc)();

    extern TaskRobot task;
    extern StateRobot state;
    extern StateFunc stateTable[];
    extern struct pt thread;

    // Объекты
    extern WallManager wallManager;
    extern WallController wallRight, wallLeft;
    extern WheelManager wheelManager;
    extern WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    extern DebugController debug;
    extern RotateController rotateController;
    extern ServoController servoController;
    extern ComController comController;

    void init();
    void loop();
    int state_update();

    void handleWait();
    void handleMove();
    void handleRotate();
    void handleVictim();

    void isr_encoder_A1(); void isr_encoder_A2();
    void isr_encoder_B1(); void isr_encoder_B2();
}

#endif