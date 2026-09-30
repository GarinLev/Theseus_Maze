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
    static constexpr int SERVO_IDLE_ANGLE = 90;

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
    void update_serial();

    String serial_buffer;

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

    // Коэффициенты для проездов
    float drive_p[3]{0.15f, 0.3f, 2.5f}; // p1, p2, p_ang
    float drive_d[3]{2.0f, 3.0f, 7.5f}; // d1, d2, d_ang

    // Состояние управления
    float rpm{0.0f};
    float steer{0.0f};
    bool is_pause{false};
    bool is_last_black{false};
    uint32_t last_fast_millis{0};
    bool force_stop{false}; 

    // Типы очередей:
    using MoveQueue   = TaskArenaStack<10, 80>; // Буфер 800 байт (под тяжелые TaskMove с 3x PID)
    using VictimQueue = TaskArenaStack<10, 32>; // Буфер 320 байт (под сервы/толкатели)

    // Очереди задач
    MoveQueue   tasks_move;
    VictimQueue tasks_victim;

    // Алиас для удобства (если в новом коде захотите обращаться как tasks_dispenser)
    VictimQueue& tasks_dispenser = tasks_victim;

private:
    Robot();
    static void tramp_slow();
    static void tramp_fast();
};

#endif // FIRMWARE_ROBOT_H