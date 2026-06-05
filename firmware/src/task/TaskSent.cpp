#include "Robot.h"
#include "Task.h"

void TaskSent::on_init() {

}

void TaskSent::on_execute() {
    auto& robot = Robot::instance();

    float dist_u = robot.dist_up.get();
    bool is_up = dist_u < 200 && dist_u != 0;

    float dist_r = robot.dist_right.get();
    bool is_right = dist_r < 200 && dist_r != 0;

    float dist_d = robot.dist_down.get();
    bool is_down = dist_d < 200 && dist_d != 0;

    float dist_l = robot.dist_left.get();
    bool is_left = dist_l < 200 && dist_l != 0;


    bool dist_array[4] = {is_up, is_right, is_down, is_left};
    robot.link.send_sensors(dist_array, robot.is_last_black ? 2 : 0);

    done();
}

