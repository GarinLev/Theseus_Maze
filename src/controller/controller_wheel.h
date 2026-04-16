#pragma once
#include <Arduino.h>
#include "../../lib/pt/pt.h"
#include "../node/nodes.h"

struct WheelController {
    struct pt pt;

    // Ноды и топики
    MotorNode       motorNode;
    MotorTopic      motorTopic;
    EncoderNode     encoderNode;
    EncoderTopic    encoderTopic;
    MotorTargetNode motorTarget;

    PI_Node         posNode;
    PI_Topic        posTopic;
    PI_Node         speedNode;
    PI_Topic        speedTopic;

    float encoder_ticks_per_rev = 500.0f;
    float max_rpm = 140.0f;
    uint32_t step_counter = 0;
    bool is_moving = false;

    void init(float tpr, float rpm) {
        PT_INIT(&pt);
        encoder_ticks_per_rev = tpr;
        max_rpm = rpm;

        posNode.topic = &posTopic;
        speedNode.topic = &speedTopic;
        motorNode.topic = &motorTopic;
        encoderNode.topic = &encoderTopic;

        speedNode.dt = 25;
        posNode.dt = 25;

        speedNode.Kp = 2.5;
        speedNode.Ki = 1.0;


        node_pi_init(posNode);
        node_pi_init(speedNode);
        node_motorT_init(motorTarget);
    }

    void setPins(int m1, int m2, bool rev, int encA, int encB, void (*isr)()) {
        motorNode.pin_in1 = (uint8_t)m1;
        motorNode.pin_in2 = (uint8_t)m2;
        motorNode.reverse = rev;
        node_motor_init(motorNode);

        encoderNode.pin_int = (uint8_t)encA;
        encoderNode.pin_dir = (uint8_t)encB;
        encoderNode.func = isr;
        node_encoder_init(encoderNode);
    }

    void setProfile(float x1, float x0, float a, float vmax) {
        motorTarget.soft.x0 = x0;
        motorTarget.soft.x1 = x1;
        motorTarget.soft.a = a;
        motorTarget.soft.Vmax = vmax;

        step_counter = 0;
        is_moving = true;

        encoderTopic.value = 0;

        posNode.integral = 0;
        speedNode.integral = 0;
    }

    void update() {
        // node_pi_run(posNode);
        node_pi_run(speedNode);
        node_motor_run(motorNode);

        process();
    }

    uint32_t last_serial_time = 0;

    int process() {
        PT_BEGIN(&pt);

        for (;;) {
            PT_WAIT_UNTIL(&pt, (uint32_t)(millis() - motorTarget.last_time) >= 25);
            motorTarget.last_time = millis();

            if (is_moving) {
                float ideal_pos = SoftGet(&motorTarget.soft, (float)step_counter++);

                posTopic.value = (float)encoderTopic.value;
                posNode.setpoint = (int16_t)ideal_pos;
                // float target_rpm = constrain(posNode.value_out, -max_rpm, max_rpm);
                float target_rpm = constrain(120, -max_rpm, max_rpm);

                speedTopic.value = getCurrentRPM();
                speedNode.setpoint = (int16_t)target_rpm;

                int final_pwm = (int)constrain(speedNode.value_out, -255, 255);
                motorTopic.speed = final_pwm;

                if (millis() - last_serial_time >= 50)
                {
                    Serial.print(target_rpm);
                    Serial.print(",");
                    Serial.print(final_pwm);
                    Serial.print(",");
                    Serial.println(getCurrentRPM());
                    last_serial_time = millis();
                }

                float error = abs(motorTarget.soft.x1 - posTopic.value);
/*                if (step_counter > 20 && abs(target_rpm) < 1.0f && error < 10) {
                    is_moving = false;
                    motorTopic.speed = 0;
                }*/

                NOTIFY_TOPIC(&motorTopic);
            }
        }

        PT_END(&pt);
    }

    float getCurrentRPM() {
        if (encoderTopic.period > 0) {
            return (1000000.0f / (float)encoderTopic.period) / encoder_ticks_per_rev * 60.0f;
        }
        return 0.0f;
    }
};
