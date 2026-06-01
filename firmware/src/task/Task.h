#ifndef FIRMWARE_TASK_H
#define FIRMWARE_TASK_H

#include "math/PID.h"
#include "math/SpeedProfile.h"

class Robot;

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
    TaskMove(const SpeedProfile &_profile, const PID &_pid_dist, const PID &_pid_yaw, Robot* _robot)
        : speed_profile(_profile), pid_dist(_pid_dist), pid_yaw(_pid_yaw), robot(_robot) {}
    void execute() override;
private:
    SpeedProfile speed_profile;
    PID pid_dist, pid_yaw;
    float start_encoder = 0.0f;
    float yaw_now = 0.0f;
    Robot* robot;
};

class TaskRotate final : public Task {
public:
    TaskRotate(const SpeedProfile &_profile, Robot* _robot)
        : speed_profile(_profile), robot(_robot) {}
    void execute() override;
    void set_direction(float dir) { direction = dir >= 0.0f ? 1.0f : -1.0f; }
private:
    SpeedProfile speed_profile;
    float prev_yaw = 0.0f;
    float unwrapped = 0.0f;
    float direction = 1.0f;
    Robot* robot;
};

#endif // FIRMWARE_TASK_H