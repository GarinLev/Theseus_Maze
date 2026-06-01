#ifndef FIRMWARE_ROBOT_H
#define FIRMWARE_ROBOT_H

#include <Ticker.h>

#include "module/Dist.h"
#include "module/IMU.h"
#include "module/Quad.h"
#include "module/Wheel.h"
#include "task/Stack.h"
#include "task/Task.h"

class Robot {
public:
    Robot();

    static void loop_slow();
    void loop_fast();
    void update_pi();

    Ticker timer_slow, timer_fast;
    Wheel w_fr, w_fl, w_br, w_bl;
    IMU imu;
    Quad quad;
    Dist dist_left, dist_right;
    float rpm{}, steer{};
    float W_Kp, W_Ki;
    float ypr[3]{};

    StaticStack<Task, 5, 128> tasks;
    static void update_tasks();
};

extern Robot robot; // NOLINT(*-dynamic-static-initializers)

#endif //FIRMWARE_ROBOT_H
