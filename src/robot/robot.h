#pragma once

#include "../controller/controller_wheel.h"
#include "../controller/controller_wall.h"

namespace robot {
    extern WallController wallLeft;
    // extern WallController wallRight;
    
    extern WheelController wheelA1;
    extern WheelController wheelA2;
    extern WheelController wheelB1;
    extern WheelController wheelB2;

    void init();
    void loop();

    void isr_encoder_A1();
    void isr_encoder_A2();
    void isr_encoder_B1();
    void isr_encoder_B2();
}
