#ifndef FIRMWARE_TASK_H
#define FIRMWARE_TASK_H

#include "math/PID.h"
#include "math/SpeedProfile.h"
#include <stdint.h>

enum class State {
    INIT,
    EXECUTING,
    DONE
};

class Task {
public:
    virtual ~Task() = default;
    void execute() {
        if (state == State::INIT) {
            on_init();
            state = State::EXECUTING;
        }

        if (state == State::EXECUTING) {
            on_execute();
        }
    };
    State state = State::INIT;

protected:
    virtual void on_init() {}
    virtual void on_execute() = 0;
    void done() { state = State::DONE; }
};

class TaskMove final : public Task {
public:
    enum class Step : uint8_t {
        DRIVE,
        BACK,
        STOP
    };

    TaskMove(const SpeedProfile &_profile, const PID &_pid_dist, const PID &_pid_yaw)
        : speed_profile(_profile), pid_dist(_pid_dist), pid_yaw(_pid_yaw) {}
    void on_execute() override;

private:
    void on_init() override;
    SpeedProfile speed_profile;
    PID pid_dist, pid_yaw;
    float start_encoder = 0.0f;
    float yaw_now = 0.0f;
    float last_encoder = 0.0f;
    float progress_encoder = 0.0f;
    Step step = Step::DRIVE;
    float back_start_encoder = 0.0f;
};

class TaskRotate final : public Task {
public:
    explicit TaskRotate(const SpeedProfile &_profile)
        : speed_profile(_profile) {}

    void on_execute() override;
    void set_direction(float dir) { direction = dir >= 0.0f ? 1.0f : -1.0f; }

private:
    void on_init() override;
    SpeedProfile speed_profile;
    float prev_yaw = 0.0f;
    float unwrapped = 0.0f;
    float direction = 1.0f;
};

class TaskTouch final : public Task {
public:
    explicit TaskTouch(float _back_ticks)
        : back_ticks(_back_ticks) {}
    void on_execute() override;

private:
    void on_init() override;
    enum class Step { INIT, SEARCH, ALIGN, BACK };
    Step step = Step::INIT;
    uint32_t step_timer = 0;
    float start_encoder = 0.0f;
    float back_ticks = 0.0f;
};

class TaskDelay final : public Task {
public:
    explicit TaskDelay(uint32_t _delay_ms) : delay_ms(_delay_ms) {}
    void on_execute() override;

private:
    void on_init() override;
    uint32_t delay_ms;
    uint32_t start_time = 0;
};

class TaskSent final : public Task {
public:
    explicit TaskSent() = default;
    void on_execute() override;

private:
    void on_init() override;
};

#endif // FIRMWARE_TASK_H
