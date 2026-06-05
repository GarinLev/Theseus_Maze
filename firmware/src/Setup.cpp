#include "Robot.h"

#include "Log.h"

Robot& Robot::instance() {
    static Robot inst;
    return inst;
}

void Robot::tramp_slow() { instance().loop_slow(); }
void Robot::tramp_fast() { instance().loop_fast(); }

Robot::Robot()
    : timer_slow(Ticker(tramp_slow, 125, 0, MILLIS)),
      timer_fast(Ticker(tramp_fast, 10, 0, MILLIS)),
      link(&Serial2, &Serial),
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
      dist_down(35, 0x35),
      dist_pop_l(37, 0x37),
      dist_pop_r(33, 0x33),
      button(42),
      touch_pin_r(40),
      touch_pin_l(41) {}

void setup() {
    auto& robot = Robot::instance();

    Serial.begin(115200);
    Serial2.begin(9600);
    Wire.begin();
    // Wire.setClock(400000L);

    LOG_INFO("Robot Setup Waiting");

    robot.led.setBrightness(255);
    robot.led.fill(mBlue);
    robot.led.show();

    robot.w_fr.init();
    robot.w_fl.init();
    robot.w_br.init();
    robot.w_bl.init();

    robot.imu.init();

    robot.dist_left.init();
    robot.dist_right.init();
    robot.dist_up.init();
    robot.dist_down.init();
    robot.dist_pop_r.init();
    robot.dist_pop_l.init();

    robot.color.init();

    robot.dist_right.write_address();
    robot.dist_left.write_address();
    robot.dist_up.write_address();
    robot.dist_down.write_address();
    robot.dist_pop_r.write_address();
    robot.dist_pop_l.write_address();

    pinMode(robot.touch_pin_l, INPUT_PULLUP);
    pinMode(robot.touch_pin_r, INPUT_PULLUP);

    robot.servo.attach(44);
    robot.servo.write(70);

    robot.timer_slow.start();
    robot.timer_fast.start();

    LOG_INFO("Robot Setup Successful");
    LOG_INFO("Robot Link Waiting");

    robot.led.fill(mAqua);
    robot.led.setBrightness(64);
    robot.led.show();
    while (!robot.button.click()) {
        robot.button.tick();
    }
    robot.led.setBrightness(255);
    robot.led.fill(mMagenta);
    robot.led.show();
    delay(100);
    robot.led.clear(); robot.led.show();

    robot.link.wait_start();

    LOG_INFO("Robot Link Successful");

    robot.tasks.push( TaskSent() );
    robot.tasks.push(TaskDelay(5000));
}