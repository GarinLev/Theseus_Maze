#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include "../../lib/pt/pt.h"

#ifndef WC_MM_TO_TICKS
#define WC_Radius 70 / 2.0f
#define WC_TICKS_PER_REV 500.0f
#define WC_MM_TO_TICKS(mm) ((mm) * WC_TICKS_PER_REV / (TWO_PI * WC_Radius))
#endif

#include "../controller/controller_wheel.h"
#include "../controller/controller_wall.h"
#include "../controller/manager_wall.h"
#include "../controller/manager_wheel.h"
#include "../controller/controller_rotate.h"
#include "../controller/controller_servo.h"
#include "../controller/controller_debug.h"
#include "../controller/controller_color.h"
#include "../../lib/EncButton/EncButton.h"
#include "../../lib/microLED/microLED.h"

// #define COM_ENABLE 1

struct ComController;

namespace robot {


    enum TaskRobot {
        TaskRobot_END,
        TaskRobot_WAIT,
        TaskRobot_STEP_UP,
        TaskRobot_STEP_LEFT,
        TaskRobot_STEP_RIGHT,
        TaskRobot_STEP_DOWN,
        TaskRobot_VICTIM_LEFT,
        TaskRobot_VICTIM_RIGHT,
        TaskRobot_VICTIM_LEFT_X2,
        TaskRobot_VICTIM_RIGHT_X2,
    };

    enum StateRobot {
        StateRobot_WAIT,
        StateRobot_MOVE,
        StateRobot_ROTATE,
        StateRobot_VICTIM
    };

    extern TaskRobot task;
    extern StateRobot state;

    extern WallManager wallManager;
    extern WallController wallRight, wallLeft;
    extern WheelManager wheelManager;
    extern WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    extern DebugController debug;
    extern RotateController rotateController;
    extern ServoController servoController;
    extern microLED<11, 43, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER, SAVE_MILLIS> strip;
    extern ColorController colorController;

    extern const int16_t OFFSETS_MPU[6] PROGMEM;
    extern const float   OFFSETS_COLOR[8] PROGMEM;

#ifdef COM_ENABLE
    extern ComController comController;
#endif

    void init();
    void loop();
    void calibrate();

    void update_step();
    void update_victim();

    void handleWait();
    void handleMove();
    void handleRotate();
    void handleVictim();

    namespace encoder {
        void isr_A1(); void isr_A2();
        void isr_B1(); void isr_B2();
    }
}
#endif