#ifndef FIRMWARE_ROBOT_H
#define FIRMWARE_ROBOT_H

#include <Ticker.h>

#include "module/Dist.h"
#include "module/IMU.h"
#include "module/Quad.h"
#include "module/Wheel.h"
#include "task/Stack.h"
#include "task/Task.h"
#include "module/Color.h"

class Robot {
public:
    Robot();

    static void loop_slow();
    void loop_fast();
    static bool touch_is();

    Ticker timer_slow, timer_fast;
    Wheel w_fr, w_fl, w_br, w_bl;
    IMU imu;
    Quad quad;
    Dist dist_left, dist_right, dist_up;
    Color color;

    float rpm{}, steer{};

    bool touch_state = false;

    uint8_t touch_pin_r, touch_pin_l;

    StaticStack<Task, 15, 128> tasks;
    static void update_tasks();
};

extern Robot robot; // NOLINT(*-dynamic-static-initializers)

#endif //FIRMWARE_ROBOT_H
