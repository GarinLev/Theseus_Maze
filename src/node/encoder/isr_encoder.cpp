#include "../../robot/robot.h"

#include <Arduino.h>
#include "../../../lib/GyverIO/GyverIO.h"

namespace robot {

void isr_encoder_A1() {
    uint32_t now = micros();
    wheelA1.encoderTopic.period = now - wheelA1.encoderNode.timer;
    
    if (gio::read(wheelA1.encoderNode.pin_dir)) {
        wheelA1.encoderTopic.value--;
    } else {
        wheelA1.encoderTopic.value++;
    }
    
    wheelA1.encoderNode.timer = now;
    NOTIFY_TOPIC(&wheelA1.encoderTopic);
}

/*void isr_encoder_A2() {
    uint32_t now = micros();
    wheelA2.encoderTopic.period = now - wheelA2.encoderNode.timer;
    
    if (gio::read(wheelA2.encoderNode.pin_dir)) {
        wheelA2.encoderTopic.value--;
    } else {
        wheelA2.encoderTopic.value++;
    }
    
    wheelA2.encoderNode.timer = now;
    NOTIFY_TOPIC(&wheelA2.encoderTopic);
}

void isr_encoder_B1() {
    uint32_t now = micros();
    wheelB1.encoderTopic.period = now - wheelB1.encoderNode.timer;
    
    if (gio::read(wheelB1.encoderNode.pin_dir)) {
        wheelB1.encoderTopic.value--;
    } else {
        wheelB1.encoderTopic.value++;
    }
    
    wheelB1.encoderNode.timer = now;
    NOTIFY_TOPIC(&wheelB1.encoderTopic);
}

void isr_encoder_B2() {
    uint32_t now = micros();
    wheelB2.encoderTopic.period = now - wheelB2.encoderNode.timer;
    
    if (gio::read(wheelB2.encoderNode.pin_dir)) {
        wheelB2.encoderTopic.value--;
    } else {
        wheelB2.encoderTopic.value++;
    }
    
    wheelB2.encoderNode.timer = now;
    NOTIFY_TOPIC(&wheelB2.encoderTopic);
}*/

} // namespace robot
