#include "Robot.h"
#include <GyverWDT.h>
#include "Log.h"

void Robot::loop_slow() {
    link.update();
    link.update_debug();
    color.update();
}

void Robot::loop_fast() {
    Watchdog.reset();
    delta_fast.start();

    const uint32_t now = millis();
    const uint32_t dt = now - last_fast_millis;
    last_fast_millis = now;

    button.tick();
    imu.update();

    dist_left.update();
    dist_right.update();
    dist_up.update();
    dist_down.update();
    dist_ang_left.update();
    dist_ang_right.update();

    w_fr.update_sensors();
    w_fl.update_sensors();
    w_br.update_sensors();
    w_bl.update_sensors();

    update_pause();
    link.update_command_queue();

    // Выполнение текущей задачи
    if (!tasks_victim.isEmpty()) {
        tasks_victim.top()->execute(dt);
    } else if (!tasks_move.isEmpty()) {
        tasks_move.top()->execute(dt);
    } else {
        servo.write(SERVO_IDLE_ANGLE);
        rpm = 0.0f;
        steer = 0.0f;
        force_stop = false;
    }

    const float target_speed = fabsf(rpm);
    w_fr.update_pi(target_speed);
    w_fl.update_pi(target_speed);
    w_br.update_pi(target_speed);
    w_bl.update_pi(target_speed);

    quad.rpm(rpm, steer);
    update_tasks();

    delta_fast.stop();
}

void Robot::reset() {
    rpm = 0.0f;
    steer = 0.0f;
    quad.rpm(0.0f, 0.0f);
    servo.write(SERVO_IDLE_ANGLE);
    led.clear();
    led.show();
}

void Robot::update_tasks() {
    if (!tasks_move.isEmpty()) {
        const Task* task_move = tasks_move.top();
        if (task_move->state == State::DONE && !is_pause) {
            LOG_INFO("Task ", task_move->name(), " closed");
            tasks_move.pop();
            rpm = 0.0f;
            steer = 0.0f;
        }
    }

    if (!tasks_victim.isEmpty()) {
        const Task* task_victim = tasks_victim.top();
        if (task_victim->state == State::DONE && !is_pause) {
            LOG_INFO("Task ", task_victim->name(), " closed");
            tasks_victim.pop();
            rpm = 0.0f;
            steer = 0.0f;
        }
    }
}

void Robot::update_pause() {
    if (!button.click()) {
        return;
    }
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
        led.fill(mSilver);
        servo.detach();
    } else {
        tasks_move.push(TaskSent());
        tasks_move.push(TaskDelay(1000));
        led.clear();
        servo.attach(PIN_SERVO);
        servo.write(SERVO_IDLE_ANGLE);
    }
    led.show();
    led.setBrightness(255);

    link.send_pause(is_pause);
}
