#ifndef FIRMWARE_TASK_H
#define FIRMWARE_TASK_H

#include "math/PID.h"
#include "math/SpeedProfile.h"

enum class StateTask {
    RUNNING,
    CLOSE
};

class Task {
public:
    virtual ~Task() = default;
    virtual void execute() {}
    StateTask state = StateTask::RUNNING;
    bool started = false;
};

class TaskMove final : public Task {
public:
    TaskMove(const SpeedProfile &_profile, const PID &_pid, float* _encoder_now,
            float* _yaw, float* _pwm, float* _steer)
        : speed_profile(_profile), pid(_pid), encoder_now(_encoder_now),
            yaw(_yaw), rpm(_pwm), steer(_steer) {};
    void execute() override;
private:
    SpeedProfile speed_profile;
    PID pid;
    float start_encoder = 0.0f;
    float *encoder_now;
    float yaw_now = 0.0f;
    float *yaw;
    float *rpm, *steer;
};

class TaskRotate final : public Task {
public:
    TaskRotate(const SpeedProfile &_profile, float* _yaw, float* _pwm)
        : speed_profile(_profile), yaw(_yaw), rpm(_pwm) {}
    void execute() override;
    void set_direction(float dir) { direction = (dir >= 0.0f) ? 1.0f : -1.0f; }
private:
    SpeedProfile speed_profile;
    float prev_yaw = 0.0f;
    float unwrapped = 0.0f;
    float direction = 1.0f;
    float *yaw;
    float *rpm;
};

#endif // FIRMWARE_TASK_H