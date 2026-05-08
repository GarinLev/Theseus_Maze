#include "../robot/robot.h"
#include "../controller/controller_wheel.h"
#include "../../lib/GyverIO/GyverIO.h"

namespace robot {

    void isr_encoder_A1() {
        uint32_t now = micros();
        wheelA1.encoderTopic.period = now - wheelA1.encoderNode.timer;
        if (gio::read(wheelA1.encoderNode.pin_dir)) {
            wheelA1.encoderTopic.reverse = false;
            wheelA1.encoderTopic.value--;
        }
        else {
            wheelA1.encoderTopic.value++;
            wheelA1.encoderTopic.reverse = true;
        }
        wheelA1.encoderNode.timer = now;
    }

    void isr_encoder_A2() {
        uint32_t now = micros();
        wheelA2.encoderTopic.period = now - wheelA2.encoderNode.timer;
        if (gio::read(wheelA2.encoderNode.pin_dir)) {
            wheelA2.encoderTopic.reverse = false;
            wheelA2.encoderTopic.value--;
        }
        else {
            wheelA2.encoderTopic.reverse = true;
            wheelA2.encoderTopic.value++;
        }
        wheelA2.encoderNode.timer = now;
    }

    void isr_encoder_B1() {
        uint32_t now = micros();
        wheelB1.encoderTopic.period = now - wheelB1.encoderNode.timer;
        if (gio::read(wheelB1.encoderNode.pin_dir)) {
            wheelB1.encoderTopic.reverse = true;
            wheelB1.encoderTopic.value--;
        }
        else {
            wheelB1.encoderTopic.reverse = false;
            wheelB1.encoderTopic.value++;
        }
        wheelB1.encoderNode.timer = now;
    }

    void isr_encoder_B2() {
        uint32_t now = micros();
        wheelB2.encoderTopic.period = now - wheelB2.encoderNode.timer;
        if (gio::read(wheelB2.encoderNode.pin_dir)) {
            wheelB2.encoderTopic.reverse = true;
            wheelB2.encoderTopic.value--;
        }
        else {
            wheelB2.encoderTopic.reverse = false;
            wheelB2.encoderTopic.value++;
        }
        wheelB2.encoderNode.timer = now;
    }

} // namespace robot