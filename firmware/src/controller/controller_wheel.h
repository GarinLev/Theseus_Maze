#pragma once
#include <Arduino.h>
#include "../../lib/pt/pt.h"
#include "../node/nodes.h"
#include "../soft/soft_move.h"

#define WC_Radius 70 / 2.0f
#define WC_TICKS_PER_REV 500.0f
#define WC_MM_TO_TICKS(mm) ((mm) * WC_TICKS_PER_REV / (TWO_PI * WC_Radius))

struct WheelController {
    struct pt pt_control;
    struct pt pt_task;

    enum States {
        STOP,
        SET,
        SOFT
    };
    States state = STOP;

    MotorNode       motorNode;
    MotorTopic      motorTopic;
    EncoderNode     encoderNode;
    EncoderTopic    encoderTopic;
    PI_Node         speedNode;
    PI_Topic        speedTopic;

    int16_t         speed_offset = 0;
    int16_t         base_rpm = 0;

    SoftMove        profile;
    uint32_t        timer = 0;
    float           max_rpm = 140.0f;
    float           encoder_ticks_per_rev = 500.0f;
    float           min_start_rpm = 50.0f;
    bool            is_moving = false;

    float    _target_v;
    uint32_t _accel_ticks;
    uint32_t _total_ticks;
    float    _start_pos;

    void stop() {
        state = STOP;
        is_moving = false;
        speed_offset = 0;
    }

    void setSpeed(int16_t rpm) {
        state = SET;
        base_rpm = rpm;
        is_moving = true;
    }

    void init(float tpr, float rpm, float min_rpm = 50.0f) {
        PT_INIT(&pt_control);
        PT_INIT(&pt_task);

        encoder_ticks_per_rev = tpr;
        max_rpm = rpm;
        min_start_rpm = min_rpm;

        speedNode.topic = &speedTopic;
        motorNode.topic = &motorTopic;
        encoderNode.topic = &encoderTopic;

        speedNode.dt = 25;
        speedNode.Kp = 2.5;
        speedNode.Ki = 5;

        node_pi_init(speedNode);
    }

    void goTo(float target_v, uint32_t accel_ticks, uint32_t total_ticks) {
        _target_v = target_v;
        _accel_ticks = accel_ticks;
        _total_ticks = total_ticks;
        _start_pos = (float)encoderTopic.value;

        state = SOFT;
        PT_INIT(&pt_task);
        is_moving = true;
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

    void update() {
        node_pi_run(speedNode);
        node_motor_run(motorNode);
        control_process();
        task_process();
    }

    int control_process() {
        PT_BEGIN(&pt_control);
        for (;;) {
            PT_WAIT_UNTIL(&pt_control, (uint32_t)(millis() - timer) >= 25);
            timer = millis();

            speedTopic.value = getCurrentRPM();

            if (state == STOP) {
                speedNode.setpoint = 0;
                speedNode.value_out = 0;
                speedNode.integral = 0;
                motorTopic.speed = 0;
            }
            else if (state == SET) {
                float target = (float)base_rpm + (float)speed_offset;
                speedNode.setpoint = (int16_t)constrain(target, -max_rpm, max_rpm);
                motorTopic.speed = (int)constrain(speedNode.value_out, -255, 255);
            }
            else if (state == SOFT) {
                float profile_rpm = SoftMoveGet(&profile, (float)encoderTopic.value);
                float target = profile_rpm + (float)speed_offset;

                speedNode.setpoint = (int16_t)constrain(target, -max_rpm, max_rpm);
                motorTopic.speed = (int)constrain(speedNode.value_out, -255, 255);
            }

            NOTIFY_TOPIC(&motorTopic);
        }
        PT_END(&pt_control);
    }

    int task_process() {
        PT_BEGIN(&pt_task);

        if (state == SOFT) {
            setMove(_start_pos, _total_ticks, _accel_ticks, _target_v);

            PT_WAIT_UNTIL(&pt_task, abs(encoderTopic.value - _start_pos) >= (_total_ticks - 10) || state == STOP);

            state = STOP;

            digitalWrite(motorNode.pin_in1, HIGH);
            digitalWrite(motorNode.pin_in2, HIGH);
        }

        PT_END(&pt_task);
    }

    void setMove(float start_pos, float total_dist, float acc_dist, float target_v) {
        profile.x_start = start_pos;
        profile.L_total = total_dist;
        profile.d_acc = acc_dist;
        profile.y0 = min_start_rpm;
        profile.y1 = target_v;
    }

    float getCurrentRPM() {
        if (micros() - encoderNode.timer > 100000) return 0.0f;
        if (encoderTopic.period > 0) {
            float abs_rpm = (1000000.0f / (float)encoderTopic.period) / encoder_ticks_per_rev * 60.0f;
            return encoderTopic.reverse ? abs_rpm : -abs_rpm;
        }
        return 0.0f;
    }
};