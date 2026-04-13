#pragma once

#include <Arduino.h>
#include <math.h>
#include "../node/nodes.h"
#include "../macro.h"      

struct WheelController {
    MotorTopic    motorTopic;
    MotorNode     motorNode;
    EncoderTopic  encoderTopic;
    EncoderNode   encoderNode;
    PI_Topic      piTopic;
    PI_Node       piNode;
    SoftMove      softMove;

    float encoder_ticks_per_rev;
    float max_rpm;

    void init(float tpr = 500.0f, float rpm = 140.0f) {
        motorNode.topic = &motorTopic;
        encoderNode.topic = &encoderTopic;
        piNode.topic = &piTopic;
        encoder_ticks_per_rev = tpr;
        max_rpm = rpm;
    }

    void setMotor(uint8_t in1, uint8_t in2, bool rev) {
        motorNode.pin_in1 = in1;
        motorNode.pin_in2 = in2;
        motorNode.reverse = rev;
        node_motor_init(motorNode);
        motorTopic.speed = 0;
        NOTIFY_TOPIC(&motorTopic);
    }

    void setEncoder(uint8_t p_int, uint8_t p_dir, void (*f)()) {
        encoderNode.pin_int = p_int;
        encoderNode.pin_dir = p_dir;
        encoderNode.func = f;
        node_encoder_init(encoderNode);
    }

    void setPI(float kp, float ki, int8_t dt = 10) {
        piNode.Kp = kp;
        piNode.Ki = ki;
        piNode.dt = dt;
        piNode.setpoint = 0;
        node_pi_init(piNode);
    }
    
    void setEncoderPins(uint8_t pin_dir) {
        encoderNode.pin_dir = pin_dir;
        encoderNode.timer = micros();
    }

    float getCurrentRPM() {
        if (encoderTopic.period > 0) {
            return (1000000.0f / (float)encoderTopic.period) / encoder_ticks_per_rev * 60.0f;
        }
        return 0.0f;
    }

    void setSpeed(float target_rpm) {
        float current_rpm = getCurrentRPM();
        piNode.setpoint = (int16_t)target_rpm;

        float error = target_rpm - current_rpm;
        if (fabsf(error) > 20.0f) {
            piNode.integral = 0.0f;
        }

        node_pi_run(piNode, &current_rpm);

        float base_pwm = (target_rpm / max_rpm) * 255.0f;
        float output_pwm = base_pwm + piTopic.value;

        if (output_pwm > 255.0f) output_pwm = 255.0f;
        if (output_pwm < 0.0f) output_pwm = 0.0f;

        motorTopic.speed = (int)output_pwm;
        NOTIFY_TOPIC(&motorTopic);
        node_motor_run(motorNode);
    }
};
