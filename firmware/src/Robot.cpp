#include "Robot.h"

#include "GyverWDT.h"
#include "Log.h"

void loop() {
    auto& robot = Robot::instance();
    robot.timer_slow.update();
    robot.timer_fast.update();
}

void Robot::loop_slow() {
    link.update();
    link.update_debug();
    color.update();
}

void Robot::loop_fast() {
    Watchdog.reset();
    delta_fast.start();

    uint32_t now = millis();
    uint32_t dt = now - last_fast_millis;
    last_fast_millis = now;

    button.tick();
    imu.update();
    dist_left.update();
    dist_right.update();
    dist_up.update();
    dist_down.update();

    w_fr.update_sensors();
    w_fl.update_sensors();
    w_br.update_sensors();
    w_bl.update_sensors();

    update_pause();

    rpm = 0;
    steer = 0;

    if (!tasks_move.isEmpty() || !tasks_victim.isEmpty()) {
        if (tasks_victim.isEmpty()) {
            tasks_move.top().execute(dt);
        } else {
            tasks_victim.top().execute(dt);
        }
    } else {
        servo.write(70);
    }

    float target_speed_pi = fabsf(rpm);
    w_fr.update_pi(target_speed_pi);
    w_fl.update_pi(target_speed_pi);
    w_br.update_pi(target_speed_pi);
    w_bl.update_pi(target_speed_pi);

    quad.rpm(rpm, steer);
    update_tasks();

    delta_fast.stop();
}

void Robot::reset() {
    rpm = 0;
    steer = 0;
    quad.rpm(0, 0);
    servo.write(70);
    led.clear();
    led.show();
}

void Robot::update_tasks() const {
    auto& r = instance();

    if (!r.tasks_move.isEmpty()) {
        const Task& task_move = r.tasks_move.top();
        if (task_move.state == State::DONE && !is_pause) {
            LOG_INFO("Task ", task_move.name(), " closed");
            r.tasks_move.pop();
        }
    }

    if (!r.tasks_victim.isEmpty()) {
        const Task& task_victim = r.tasks_victim.top();
        if (task_victim.state == State::DONE && !is_pause) {
            LOG_INFO("Task ", task_victim.name(), " closed");
            r.tasks_victim.pop();
        }
    }
}

void Robot::update_pause() {
    if (button.click()) {
        is_pause = !is_pause;

        tasks_move.clear();
        tasks_victim.clear();

        LOG_INFO(is_pause ? "Pause start" : "Pause end");

        led.setBrightness(255);
        led.fill(mMagenta);
        led.show();
        delay(100);

        led.setBrightness(64);
        if (is_pause) {
            led.fill    (mSilver);
            servo.detach();
        } else {
            led.clear();
            servo.attach(44);
            servo.write(70);
        }
        led.show();
        led.setBrightness(255);


        link.pause(is_pause);
    }
}
