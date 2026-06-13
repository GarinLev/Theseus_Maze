#ifndef FIRMWARE_ROBOT_H
#define FIRMWARE_ROBOT_H

#include <Ticker.h>

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
    static bool touch_is();

    Ticker timer_slow, timer_fast;
    Link link;
    Wheel w_fr, w_fl, w_br, w_bl;
    IMU imu;
    Quad quad;
    Dist dist_left, dist_right, dist_up, dist_down;
    Dist dist_pop_l, dist_pop_r;
    Color color;

    float rpm{}, steer{};

    bool touch_state = false;

    uint8_t touch_pin_r, touch_pin_l;

    StaticStack<Task, 12, 128> tasks;
    static void update_tasks();

private:
    static void tramp_slow();
    static void tramp_fast();
    Robot();
};

#endif //FIRMWARE_ROBOT_H
