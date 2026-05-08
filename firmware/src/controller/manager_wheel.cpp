#include "controller_wheel.h"

void WheelController::stop() {
    state = STOP;
    is_moving = false;
    speed_offset = 0;
}

void WheelController::setSpeed(int16_t rpm) {
    state = SET;
    base_rpm = rpm;
    is_moving = true;
}

void WheelController::init(float tpr, float rpm, float min_rpm = 35.0f) {
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

void WheelController::goTo(float target_v, uint32_t accel_ticks, int32_t total_ticks, float* yaw) {
    _target_v = target_v;
    _accel_ticks = accel_ticks;
    _total_ticks = abs(total_ticks);
    _dir = (total_ticks >= 0) ? 1.0f : -1.0f;
    _anegel_mpu = yaw;
    _start_angle = (yaw != nullptr) ? *yaw : 0;
    _start_pos = (float)encoderTopic.value;
    _last_enc = _start_pos;
    _real_enc = 0;
    state = SOFT;
    PT_INIT(&pt_task);
    is_moving = true;
}

void WheelController::setPins(int m1, int m2, bool rev, int encA, int encB, void (*isr)()) {
    motorNode.pin_in1 = (uint8_t)m1;
    motorNode.pin_in2 = (uint8_t)m2;
    motorNode.reverse = rev;
    node_motor_init(motorNode);
    encoderNode.pin_int = (uint8_t)encA;
    encoderNode.pin_dir = (uint8_t)encB;
    encoderNode.func = isr;
    node_encoder_init(encoderNode);
}

void WheelController::update() {
    node_pi_run(speedNode);
    node_motor_run(motorNode);
    control_process();
    task_process();
}

int WheelController::control_process() {
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
            float current_angle = (_anegel_mpu != nullptr) ? *_anegel_mpu : 0;
            float angle_rad = (current_angle - _start_angle) * DEG_TO_RAD;
            float delta_enc = (float)encoderTopic.value - _last_enc;
            _real_enc += delta_enc * cos(angle_rad);
            _last_enc = (float)encoderTopic.value;

            float profile_rpm = SoftMoveGet(&profile, abs(_real_enc));
            float target = (profile_rpm * _dir) + (float)speed_offset;
            speedNode.setpoint = (int16_t)constrain(target, -max_rpm, max_rpm);
            motorTopic.speed = (int)constrain(speedNode.value_out, -255, 255);
        }
        NOTIFY_TOPIC(&motorTopic);
    }
    PT_END(&pt_control);
}

int WheelController::task_process() {
    PT_BEGIN(&pt_task);
    if (state == SOFT) {
        setMove(0, (float)_total_ticks, (float)_accel_ticks, _target_v);
        PT_WAIT_UNTIL(&pt_task, abs(_real_enc) >= ((float)_total_ticks - 10.0f) || state == STOP);

        state = STOP;
        digitalWrite(motorNode.pin_in1, HIGH);
        digitalWrite(motorNode.pin_in2, HIGH);
        is_moving = false;
    }
    PT_END(&pt_task);
}

void WheelController::setMove(float start_pos, float total_dist, float acc_dist, float target_v) {
    profile.x_start = start_pos;
    profile.L_total = total_dist;
    profile.d_acc = acc_dist;
    profile.y0 = min_start_rpm;
    profile.y1 = target_v;
}

float WheelController::getCurrentRPM() {
    if (micros() - encoderNode.timer > 100000) return 0.0f;
    if (encoderTopic.period > 0) {
        float abs_rpm = (1000000.0f / (float)encoderTopic.period) / encoder_ticks_per_rev * 60.0f;
        return encoderTopic.reverse ? abs_rpm : -abs_rpm;
    }
    return 0.0f;
}