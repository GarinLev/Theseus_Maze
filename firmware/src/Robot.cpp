#include "Robot.h"
#include "Log.h"

void loop() {
    auto& robot = Robot::instance();
    robot.timer_slow.update();
    robot.timer_fast.update();
}

void Robot::loop_slow() {
    link.update();
    color.update();
}

void Robot::loop_fast() {
    imu.update();
    dist_left.update();
    dist_right.update();
    dist_up.update();
    dist_down.update();
    w_fr.update_sensors();
    w_fl.update_sensors();
    w_br.update_sensors();
    w_bl.update_sensors();
    touch_state = digitalRead(touch_pin_r) == LOW && digitalRead(touch_pin_l) == LOW;

    float target_speed_pi = fabsf(rpm);
    w_fr.update_pi(target_speed_pi);
    w_fl.update_pi(target_speed_pi);
    w_br.update_pi(target_speed_pi);
    w_bl.update_pi(target_speed_pi);

    if (!tasks.isEmpty()) {
        tasks.top().execute();
    }

    quad.rpm(rpm, steer);
    update_tasks();
}

