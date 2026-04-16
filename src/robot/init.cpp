#include "robot.h"

namespace robot {
    WheelController wheelA1;
/*    WheelController wheelA2;
    WheelController wheelB1;
    WheelController wheelB2;*/

    void init() {
        pinMode(6, OUTPUT);
        digitalWrite(6, LOW);
        pinMode(7, OUTPUT);
        digitalWrite(7, LOW);
        pinMode(10, OUTPUT);
        digitalWrite(10, LOW);
        pinMode(12, OUTPUT);
        digitalWrite(12, LOW);
        pinMode(13, OUTPUT);
        digitalWrite(13, LOW);
        pinMode(9, OUTPUT);
        digitalWrite(9, LOW);
        pinMode(8, OUTPUT);
        digitalWrite(8, LOW);
        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);

        wheelA1.init(500.0f, 140.0f);
        wheelA1.setPins(10, 12, false, 18, 24, robot::isr_encoder_A1);
        wheelA1.setProfile(2500, 0, 0.5f, 15.0f);

/*
        wheelA2.init(500.0f, 140.0f);
        wheelA2.setPins(10, 12, false, 18, 24, robot::isr_encoder_A2);

        wheelB1.init(500.0f, 140.0f);
        wheelB1.setPins(13, 9, true, 3, 23, robot::isr_encoder_B1);

        wheelB2.init(500.0f, 140.0f);
        wheelB2.setPins(8, 4, true, 19, 25, robot::isr_encoder_B2);*/

    }
}
