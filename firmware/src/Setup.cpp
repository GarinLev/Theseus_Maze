#include "Robot.h"

#include "Log.h"

Robot robot;

static TaskMove get_task_move() {
    return TaskMove(
        SpeedProfile(30, 70, Quad_MM(300), Quad_MM(150), Quad_MM(50)),
        PID( 1.1, 0, 0.1, -30, 30 ),
        &robot.encoder, &robot.ypr[0], &robot.rpm, &robot.steer
    );
}

Robot::Robot()
    : timer_slow(Ticker([] { robot.loop_slow(); }, 250, 0, MILLIS)),
    timer_fast(Ticker([] { robot.loop_fast(); }, 10, 0, MILLIS)),
    w_fr(4, 5, 2, 22, false, enc_fr,
        2.5f, 2.0f, 0, -255.0f, 255.0f),
    w_fl(8, 9, 18, 24, false, enc_fl,
        2.5f, 2.0f, 0, -255.0f, 255.0f),
    w_br(6, 7, 3, 23, true, enc_br,
        2.5f, 2.0f, 0, -255.0f, 255.0f),
    w_bl(10, 12, 19, 25, true, enc_bl,
        2.5f, 2.0f, 0, -255.0f, 255.0f),
    quad(&w_fr, &w_fl, &w_br, &w_bl),
    dist(32, 0x32),
    rpm(0.0f),
    steer(0.0f),
    W_Kp(2.5f),
    W_Ki(2.0f),
    ypr{0.0f, 0.0f, 0.0f},
    encoder(0)
{}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000L);

    robot.w_fr.init();
    robot.w_fl.init();
    robot.w_br.init();
    robot.w_bl.init();

    robot.imu.init();
    robot.dist.init();
    robot.dist.write_address();

    robot.timer_slow.start();
    robot.timer_fast.start();

    /*
    auto test_task = TaskMove(SpeedProfile(30, 70, Quad_MM(300), Quad_MM(150), Quad_MM(50)),
        &robot.encoder, &robot.rpm);
    robot.tasks.push(test_task);
    LOG_INFO("Test task added");
    */


    LOG_INFO("Robot Setup Successful");
}

void Robot::update_pi() {
    float target = rpm + steer;
    if (target < 20) target = 20;

    W_Kp = 0.09 * target + 0.7;
    W_Ki = 0.06 * target + 0.8;
}

void Robot::update_encoder() {
    encoder = quad.encoder();
}

void Robot::update_tasks() {
    if (robot.tasks.isEmpty()) return;

    Task& task = robot.tasks.top();
    if (task.state == StateTask::CLOSE) {
        LOG_INFO("Task closed");
        robot.tasks.pop();
    }
}

