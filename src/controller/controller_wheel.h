#pragma once

#include <Arduino.h>
#include <math.h>
#include "../node/nodes.h"
#include "../macro.h"      

struct WheelController {
    MotorTopic        motorTopic;
    MotorNode         motorNode;
    MotorTopicTarget  motorTargetTopicOut;
    MotorTargetNode   motorTarget;
    EncoderTopic      encoderTopic;
    EncoderNode       encoderNode;
    PI_Topic          piTopic;
    PI_Node           piNode;

    float encoder_ticks_per_rev;
    float max_rpm;
    float current_val = (float)encoderNode.topic->value;

    void init(float tpr = 500.0f, float rpm = 140.0f) {
        motorNode.topic = &motorTopic;
        encoderNode.topic = &encoderTopic;
        motorTarget.topic_out = &motorTargetTopicOut;
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

    float getCurrentRPM() const {
        if (encoderTopic.period > 0) {
            return (1000000.0f / (float)encoderTopic.period) / encoder_ticks_per_rev * 60.0f;
        }
        return 0.0f;
    }

    void updatePI(float target_rpm) {
        piNode.setpoint = (int16_t)target_rpm;

        motorTopic.speed = (int)piNode.value_out;
        NOTIFY_TOPIC(&motorTopic);

        if (millis() % 250 == 0) {
            Serial.println(String(piNode.value_out) + "," + String(target_rpm));
        }
    }


    void notifyEncoderSpeed(float x0, float x1, float a, float Vmax) {
        motorTarget.soft.x0 = x0;
        motorTarget.soft.x1 = x1;
        motorTarget.soft.a = a;
        motorTarget.soft.Vmax = Vmax;
    }

    void update() {
        updatePI(motorTargetTopicOut.speed);
        node_pi_run(piNode);
        node_motor_run(motorNode);
        node_motorT_run(motorTarget, encoderNode.topic->value);
    }
};
