#include "Robot.h"
#include "Task.h"

void TaskSent::on_init() {
    zero_count_up = 0;
    zero_count_right = 0;
    zero_count_down = 0;
    zero_count_left = 0;
    cycle_count = 0;
}

void TaskSent::on_execute(uint32_t dt) {
    auto& robot = Robot::instance();

    float dist_u = robot.dist_up.get();
    float dist_r = robot.dist_right.get();
    float dist_d = robot.dist_down.get();
    float dist_l = robot.dist_left.get();

    if (dist_u >= 200 || dist_u == 0) zero_count_up++;
    if (dist_r >= 200 || dist_r == 0) zero_count_right++;
    if (dist_d >= 200 || dist_d == 0) zero_count_down++;
    if (dist_l >= 200 || dist_l == 0) zero_count_left++;

    cycle_count++;

    if (cycle_count < 5) {
        return;
    }

    bool is_up    = (zero_count_up < 2);
    bool is_right = (zero_count_right < 2);
    bool is_down  = (zero_count_down < 2);
    bool is_left  = (zero_count_left < 2);

    uint8_t num_sent = 0;
    if (robot.color.get_current_color() == COLOR_SILVER) num_sent = 1;
    if (robot.is_last_black) {
        robot.is_last_black = false;
        num_sent = 2;
    }

    bool dist_array[4] = {is_up, is_right, is_down, is_left};
    robot.link.send_sensors(dist_array, num_sent);

    done();
}