#pragma once
#include <Arduino.h>
#include "../../lib/pt/pt.h"
#include "../node/nodes.h"
#include "../soft/soft_move.h"

struct WheelController {
    struct pt pt_control, pt_task;
    enum States { STOP, SET, SOFT } state = STOP;

    MotorNode motorNode; MotorTopic motorTopic;
    EncoderNode encoderNode; EncoderTopic encoderTopic;
    PI_Node speedNode; PI_Topic speedTopic;

    int16_t speed_offset = 0;
    int16_t base_rpm = 0;
    SoftMove profile;
    uint32_t timer = 0;
    float max_rpm = 140.0f;
    float encoder_ticks_per_rev = 500.0f;
    float min_start_rpm = 35.0f;
    bool is_moving = false;

    float _target_v, _real_enc = 0, _last_enc = 0, _start_pos, _start_angle, _dir = 1.0f;
    uint32_t _accel_ticks, _total_ticks;
    float* _anegel_mpu = nullptr;

    void init(float tpr, float rpm, float min_rpm = 35.0f);
    void stop();
    void setSpeed(int16_t rpm);
    void goTo(float target_v, uint32_t accel_ticks, int32_t total_ticks, float* yaw);
    void setPins(int m1, int m2, bool rev, int encA, int encB, void (*isr)());
    void update();
    int control_process();
    int task_process();
    void setMove(float start_pos, float total_dist, float acc_dist, float target_v);
    float getCurrentRPM();
};