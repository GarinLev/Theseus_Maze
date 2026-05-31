#include "Wheel.h"

#include <Arduino.h>
#include "robot.h"

constexpr int SPEED_MIN = -255;
constexpr int SPEED_MAX = 255;
constexpr int ENC_TICK_REV = 500;

void Wheel::init() {
    pinMode(pin_a, OUTPUT);
    pinMode(pin_b, OUTPUT);

    pinMode(pin_encD, INPUT);
    pinMode(pin_encI, INPUT);

    sign = 1;
    enc_timer = micros();

    attachInterrupt(digitalPinToInterrupt(pin_encI),
        enc_func, RISING);
}

void Wheel::set(float speed) const {
    float constrained_speed = constrain(speed, SPEED_MIN, SPEED_MAX);

    if (receive) {
        constrained_speed = -constrained_speed;
    }

    if (constrained_speed == 0) {
        digitalWrite(pin_a, LOW);
        digitalWrite(pin_b, LOW);
    }
    else if (constrained_speed > 0) {
        digitalWrite(pin_a, LOW);
        analogWrite(pin_b, (int16_t)constrained_speed);
    }
    else {
        digitalWrite(pin_a, HIGH);
        analogWrite(pin_b, 255 - (int16_t)abs(constrained_speed));
    }
}

float Wheel::real() const {
    uint32_t local_period = enc_period;
    uint32_t local_timer = enc_timer;
    int8_t local_sign = sign;

    if (micros() - local_timer > 10000) return 0.0f;
    if (local_period < 500) return 0.0f;

    float abs_rpm = (1000000.0f / (float)local_period) / ENC_TICK_REV * 60.0f;

    return abs_rpm * local_sign;
}

void Wheel::update(float pwm) {
    float speed_now = real();
    float speed_need = _pid.compute(pwm, speed_now);

    if (pwm == 0) {
        set(0);
        _pid.reset();
    } else {
        set(-speed_need);
    }
}

void Wheel::handle_encoder_interrupt() {
    uint32_t now = micros();

    enc_period = now - enc_timer;
    enc_timer = now;

    if (digitalRead(pin_encD) ^ receive) {
        enc_value--;
        sign = -1;
    } else {
        enc_value++;
        sign = 1;
    }
}

void enc_fr() { robot.w_fr.handle_encoder_interrupt(); }
void enc_fl() { robot.w_fl.handle_encoder_interrupt(); }
void enc_br() { robot.w_br.handle_encoder_interrupt(); }
void enc_bl() { robot.w_bl.handle_encoder_interrupt(); }
