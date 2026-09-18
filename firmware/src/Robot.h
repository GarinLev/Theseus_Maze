#ifndef FIRMWARE_ROBOT_H
#define FIRMWARE_ROBOT_H

#include <Ticker.h>
#include <microLED.h>
#include <uButton.h>

#include "Servo.h"
#include "link/Link.h"
#include "delta/Delta.h"
#include "module/Color.h"
#include "module/Dist.h"
#include "module/IMU.h"
#include "module/Quad.h"
#include "module/Wheel.h"
#include "task/Stack.h"
#include "task/Task.h"

class Robot {
public:
    static constexpr uint8_t PIN_SERVO = 44;
    static constexpr int SERVO_IDLE_ANGLE = 70;

    static constexpr uint8_t PIN_TOUCH_R = 40;
    static constexpr uint8_t PIN_TOUCH_L = 41;

    static Robot& instance();

    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;

    void loop_slow();
    void loop_fast();

    void reset();
    void update_tasks();
    void update_pause();

    // Модули
    Ticker timer_slow;
    Ticker timer_fast;
    Link link;
    Wheel w_fr, w_fl, w_br, w_bl;
    IMU imu;
    Quad quad;
    Dist dist_left, dist_right, dist_up, dist_down;
    Dist dist_ang_left, dist_ang_right;
    Color color;
    Servo servo;
    uButton button;
    microLED<11, 43, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> led;
    Delta delta_fast;

    // Пины бамперов (поддерживаются оба варианта вызова)
    uint8_t touch_pin_r{PIN_TOUCH_R};
    uint8_t touch_pin_l{PIN_TOUCH_L};

    // Переменные регуляторов (нужны для TaskMove)
    float wall_pd_kp{0.3f};
    float wall_pd_kd{3.0f};
    float one_pd_kp{0.3f};
    float one_pd_kd{3.0f};

    // Состояние управления
    float rpm{0.0f};
    float steer{0.0f};
    bool is_pause{false};
    bool is_last_black{false};
    uint32_t last_fast_millis{0};
    bool force_stop{false};

    // Очереди задач
    using TaskStack = TaskArenaStack<10, TaskMove, TaskRotate, TaskTouch, TaskDelay, TaskSent, TaskPush, TaskBlue, TaskBlack, TaskHit, TaskLed, TaskExit, TaskCenter>;
    TaskStack tasks_move;
    TaskStack tasks_victim;

private:
    Robot();
    static void tramp_slow();
    static void tramp_fast();
};

#endif // FIRMWARE_ROBOT_H
