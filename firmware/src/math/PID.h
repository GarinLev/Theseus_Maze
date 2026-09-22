#ifndef FIRMWARE_PID_H
#define FIRMWARE_PID_H

class PID {
public:
    PID() = default;

    PID(float kp, float ki, float kd, float out_min, float out_max)
        : Kp(kp), Ki(ki), Kd(kd), out_min(out_min), out_max(out_max)
        , integrator(0), prev_error(0) {}

    void set_gains(float kp, float ki) { Kp = kp; Ki = ki; }
    void set_pd(float kp, float kd) { Kp = kp; Kd = kd; }
    float get_kp() const { return Kp; }
    float get_ki() const { return Ki; }
    float get_last_error() const { return last_error; }
    float get_last_setpoint() const { return last_setpoint; }

    float compute(float setpoint, float process_value);
    void reset();

private:
    float Kp = 0;
    float Ki = 0;
    float Kd = 0;
    float out_min = 0;
    float out_max = 0;
    float integrator = 0;
    float prev_error = 0;
    float last_error = 0;
    float last_setpoint = 0;
};

#endif