#ifndef FIRMWARE_TASK_H
#define FIRMWARE_TASK_H

#include "Log.h"
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
            LOG_INFO("Task ", name(), " started");
            on_init();
            state = State::EXECUTING;
        }

        if (state == State::EXECUTING) {
            on_execute();
        }
    };
    virtual const char* name() const { return "Task"; }
    State state = State::INIT;

protected:
    virtual void on_init() {}
    virtual void on_execute() = 0;
    void done() { state = State::DONE; }
};

class TaskMove final : public Task {
public:
    TaskMove(const SpeedProfile &_profile, const PID &_pid_dist, const PID &_pid_yaw)
        : speed_profile(_profile), pid_dist(_pid_dist), pid_yaw(_pid_yaw) {}
    const char* name() const override { return "TaskMove"; }
    void on_execute() override;

private:
    void on_init() override;
    SpeedProfile speed_profile;
    PID pid_dist, pid_yaw;
    float start_encoder = 0.0f;
    float yaw_now = 0.0f;
    float last_encoder = 0.0f;
    float progress_encoder = 0.0f;
    uint32_t touch_start_time = 0;
    bool touch_was_pressed = false;
};

class TaskRotate final : public Task {
public:
    explicit TaskRotate(const SpeedProfile &_profile)
        : speed_profile(_profile) {}

    const char* name() const override { return "TaskRotate"; }
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
    const char* name() const override { return "TaskTouch"; }
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
    const char* name() const override { return "TaskDelay"; }
    void on_execute() override;

private:
    void on_init() override;
    uint32_t delay_ms;
    uint32_t start_time = 0;
};

class TaskSent final : public Task {
public:
    explicit TaskSent() = default;
    const char* name() const override { return "TaskSent"; }
    void on_execute() override;

private:
    void on_init() override;
};

class TaskPush final : public Task {
public:
    enum class Mode {
        LEFT, LEFT_X2,
        RIGHT, RIGHT_X2
    };

    explicit TaskPush(Mode _mode)
        : mode(_mode),
          start_pos(70) {};

    const char* name() const override { return "TaskPush"; }
    void on_execute() override;

private:
    enum class Step : uint8_t {
        TO_TARGET,
        WAIT_TARGET,
        TO_HOME,
        WAIT_HOME
    };

    void on_init() override;

    Mode mode;
    Step step = Step::TO_TARGET;

    float current_servo_pos{};
    float target_pos{};
    float home_pos{};
    float start_pos;

    uint8_t pushes_left{};
    uint32_t wait_start_ms{};
};

class TaskBlue final : public Task {
public:
    TaskBlue() = default;
    const char* name() const override { return "TaskBlue"; }
    void on_execute() override;

private:
    void on_init() override;
    uint32_t blue_wait_start = 0;
    bool active = false;
};

class TaskBlack final : public Task {
public:
    enum class Step : uint8_t {
        BACK,
        STOP
    };

    TaskBlack() = default;
    const char* name() const override { return "TaskBlack"; }
    void on_execute() override;

private:
    void on_init() override;
    Step step = Step::BACK;
    float back_start_encoder = 0.0f;
};

class TaskHit final : public Task {
public:
    enum Mode {
        LEFT, RIGHT
    };
    explicit TaskHit(Mode _mode)
        : mode(_mode) {}

    const char* name() const override { return "TaskHit"; }
    void on_execute() override;

private:
    void on_init() override;
    Mode mode;
    float start_encoder = 0.0f;
};

class TaskLed final : public Task {
public:
    TaskLed() = default;
    const char* name() const override { return "TaskLed"; }
    void on_execute() override;

private:
    void on_init() override;
    uint32_t start_time = 0;
    uint32_t last_toggle = 0;
    bool led_on = false;
};

#endif // FIRMWARE_TASK_H
