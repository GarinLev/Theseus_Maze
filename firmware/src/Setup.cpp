#include "Robot.h"

#include "Log.h"

Robot robot;

static auto task_move = TaskMove(
        SpeedProfile(30, 100, Quad_MM(300), Quad_MM(200), Quad_MM(50)),
        PID( 0.15, 0, 0.04, -200, 200 ),
        PID( 1.1, 0, 0.1, -30, 30 ),
        &robot
    );

Robot::Robot()
    : timer_slow(Ticker([] {
        robot.loop_slow();
      }, 125, 0, MILLIS)),
      timer_fast(Ticker([] {
        robot.loop_fast();
      }, 10, 0, MILLIS)),
      w_fr(4, 5, 2, 22, false, enc_fr,
           2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_fl(8, 9, 18, 24, false, enc_fl,
           2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_br(6, 7, 3, 23, true, enc_br,
           2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_bl(10, 12, 19, 25, true, enc_bl,
           2.5f, 2.0f, 0, -255.0f, 255.0f),
      quad(&w_fr, &w_fl, &w_br, &w_bl),
      dist_left(36, 0x36),
      dist_right(34, 0x34),
      dist_up(32, 0x32),
      touch_pin_r(40),
      touch_pin_l(41) {}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000L);

    robot.w_fr.init();
    robot.w_fl.init();
    robot.w_br.init();
    robot.w_bl.init();
    robot.imu.init();

    robot.dist_left.init();
    robot.dist_right.init();
    robot.dist_up.init();

    robot.color.init();

    robot.dist_right.write_address();
    robot.dist_left.write_address();
    robot.dist_up.write_address();

    pinMode(robot.touch_pin_l, INPUT_PULLUP);
    pinMode(robot.touch_pin_r, INPUT_PULLUP);

    robot.timer_slow.start();
    robot.timer_fast.start();

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskRotate(SpeedProfile(30, 60, 90, 30, 30), &robot) );

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);

    robot.tasks.push( TaskDelay(1000) );
    robot.tasks.push( TaskTouch(&robot, Quad_MM(50)) );
    robot.tasks.push(task_move);


    LOG_INFO("Robot Setup Successful");
}

void Robot::update_tasks() {
    if (robot.tasks.isEmpty()) return;

    const Task& task = robot.tasks.top();
    if (task.state == State::DONE) {
        LOG_INFO("Task closed");
        robot.tasks.pop();
    }
}

bool Robot::touch_is() {
    return robot.touch_state;
}
