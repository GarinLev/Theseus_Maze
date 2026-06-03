#include "Robot.h"
#include "Task.h"

void TaskSent::on_init() {

}

void TaskSent::on_execute() {
    auto& robot = Robot::instance();

    bool is_up = robot.dist_up.get() < 20;
    bool is_right = robot.dist_right.get() < 20;
    bool is_down = robot.dist_down.get() < 20;
    bool is_left = robot.dist_left.get() < 20;
    uint8_t color_number = 0;
    ColorType color = robot.color.get_current_color();

    switch (color) {
        case COLOR_WHITE: {
            color_number = 0;
            break;
        }
        /*case COLOR_SILVER: {
            color_number = 1;
            break;
        }*/
        case COLOR_BLACK: {
            color_number = 2;
            break;
        }
        default: break;
    }

    bool dist_array[4] = {is_up, is_right, is_down, is_left};
    robot.link.send_sensors(dist_array, color_number);

    done();
}

