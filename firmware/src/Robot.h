#ifndef FIRMWARE_ROBOT_H
#define FIRMWARE_ROBOT_H

#include <Ticker.h>
#include <microLED.h>
#include <uButton.h>

#include "Servo.h"
#include "link/Link.h"
#include "module/Dist.h"
#include "module/IMU.h"
#include "module/Quad.h"
#include "module/Wheel.h"
#include "task/Stack.h"
#include "task/Task.h"
#include "module/Color.h"
class Robot {
public:
    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;

    static Robot& instance();

    void loop_slow();
    void loop_fast();

    Ticker timer_slow, timer_fast;
    Link link;
    Wheel w_fr, w_fl, w_br, w_bl;
    IMU imu;
    Quad quad;
    Dist dist_left, dist_right, dist_up, dist_down;
    Dist dist_pop_l, dist_pop_r;
    Color color;
    Servo servo;
    uButton button;
    microLED<11, 43, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER, SAVE_MILLIS> led;

    float rpm{}, steer{};

    uint8_t touch_pin_r, touch_pin_l;

    StaticStack<Task, 12, 128> tasks;
    void update_tasks() const;
    boolean is_pause = false;
    void update_pause();

    bool is_last_black = false;

private:
    static void tramp_slow();
    static void tramp_fast();
    Robot();
};

#endif //FIRMWARE_ROBOT_H
