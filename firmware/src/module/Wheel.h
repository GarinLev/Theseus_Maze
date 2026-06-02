#ifndef FIRMWARE_MOTOR_H
#define FIRMWARE_MOTOR_H

#include <stdint.h>
#include "../math/PID.h"

class Wheel {
public:
    Wheel() = default;

    Wheel(uint8_t pin_a, uint8_t pin_b, uint8_t pin_encI, uint8_t pin_encD,
          bool receive, void (*enc_func)(),
          float kp, float ki, float kd, float out_min, float out_max)
        : pin_a(pin_a), pin_b(pin_b), pin_encI(pin_encI), pin_encD(pin_encD)
        , receive(receive), enc_func(enc_func)
        , _pid(kp, ki, kd, out_min, out_max) {}

    void init();
    void set(float speed) const;
    void update_sensors();
    float real() const;
    void update(float pwm);
    void set_pid_gains(float kp, float ki);
    void update_pi(float target);

    int32_t get_encoder() const { return enc_value; }
    void reset_encoder() { enc_value = 0; }
    float get_kp() const { return _pid.get_kp(); }
    float get_ki() const { return _pid.get_ki(); }

    void handle_encoder_interrupt();

private:
    uint8_t pin_a = 0;
    uint8_t pin_b = 0;
    uint8_t pin_encI = 0;
    uint8_t pin_encD = 0;
    bool receive = false;
    void (*enc_func)() = nullptr;
    PID _pid;

    volatile uint32_t enc_period = 0;
    volatile uint32_t enc_timer = 0;
    volatile int32_t enc_value = 0;
    volatile int8_t sign = 1;

    float last_rpm = 0;
};

void enc_fr();
void enc_fl();
void enc_br();
void enc_bl();

#endif //FIRMWARE_MOTOR_H
