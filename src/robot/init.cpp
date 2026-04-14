#include "robot.h"

namespace robot {
    WheelController wheelA1;
    WheelController wheelA2;
    WheelController wheelB1;
    WheelController wheelB2;

    void init() {
        const float Kp = 7.0f;
        const float Ki = 2.5f;

        wheelA1.init(500.0f, 140.0f);
        wheelA1.setMotor(6, 7, false);
        wheelA1.setEncoder(2, 22, robot::isr_encoder_A1);
        wheelA1.setPI(Kp, Ki);

        wheelA2.init(500.0f, 140.0f);
        wheelA2.setMotor(10, 12, false);
        wheelA2.setEncoder(18, 24, robot::isr_encoder_A2);
        wheelA2.setPI(Kp, Ki);

        wheelB1.init(500.0f, 140.0f);
        wheelB1.setMotor(13, 9, true);
        wheelB1.setEncoder(3, 23, robot::isr_encoder_B1);
        wheelB1.setPI(Kp, Ki);

        wheelB2.init(500.0f, 140.0f);
        wheelB2.setMotor(8, 4, true);
        wheelB2.setEncoder(19, 25, robot::isr_encoder_B2);
        wheelB2.setPI(Kp, Ki);

        wheelA1.notifyEncoderSpeed(50, 80, 0.3, 0.01);

    }
}
