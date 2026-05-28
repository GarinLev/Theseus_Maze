#ifndef FIRMWARE_PID_H
#define FIRMWARE_PID_H

class PID {
public:
    PID() = default;

    PID(float kp, float ki, float kd, float out_min, float out_max)
        : Kp(kp), Ki(ki), Kd(kd), out_min(out_min), out_max(out_max)
        , integrator(0), prev_error(0) {}

    void set_gains(float kp, float ki) { Kp = kp; Ki = ki; }
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
};

#endif //FIRMWARE_PID_H
