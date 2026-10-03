#include <Arduino.h>
#include <Wire.h>
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
      w_fr(4, 5, 2, 22, false, enc_fr, 2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_fl(8, 9, 18, 24, false, enc_fl, 2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_br(6, 7, 3, 23, true,  enc_br, 2.5f, 2.0f, 0, -255.0f, 255.0f),
      w_bl(10, 12, 19, 25, true, enc_bl, 2.5f, 2.0f, 0, -255.0f, 255.0f),
      quad(&w_fr, &w_fl, &w_br, &w_bl),
      dist_left(36, 0x36),
      dist_right(34, 0x34),
      dist_up(32, 0x32),
      dist_down(35, 0x35),
      dist_ang_left(37, 0x37),
      dist_ang_right(33, 0x33),
      button(42) {}

void setup() {

    Serial.begin(115200);
    Serial.setTimeout(10);
    Serial2.begin(9600);
    Wire.begin();
    Wire.setWireTimeout(20000, true);

    LOG_INFO("Robot Setup Waiting");

    auto& robot = Robot::instance();

    robot.led.setBrightness(255);
    robot.led.fill(mBlue);
    robot.led.show();

    robot.w_fr.init();
    robot.w_fl.init();
    robot.w_br.init();
    robot.w_bl.init();

    robot.imu.init();
    robot.imu.zero();

    robot.dist_left.init();
    robot.dist_right.init();
    robot.dist_up.init();
    robot.dist_down.init();
    robot.dist_ang_right.init();
    robot.dist_ang_left.init();

    robot.dist_right.write_address();
    robot.dist_left.write_address();
    robot.dist_up.write_address();
    robot.dist_down.write_address();
    robot.dist_ang_right.write_address();
    robot.dist_ang_left.write_address();

    robot.color.init();

    pinMode(Robot::PIN_TOUCH_L, INPUT_PULLUP);
    pinMode(Robot::PIN_TOUCH_R, INPUT_PULLUP);

    robot.servo.attach(Robot::PIN_SERVO);
    robot.servo.write(Robot::SERVO_IDLE_ANGLE);

    if (robot.error_status != 0) {
        LOG_ERROR("Robot Setup Error");
        for(int i = 0; i < (5); ++i) {
            robot.led.fill(mMaroon);
            robot.led.show();
            delay(100);
            robot.led.fill(mWhite);
            robot.led.show();
            delay(100);
        }
        robot.led.clear();
        robot.led.show();
    }


    robot.timer_slow.start();
    robot.timer_fast.start();

    LOG_INFO("Robot Setup Successful");
    LOG_INFO("Robot Link Waiting");


    robot.led.fill(mAqua);
    robot.led.setBrightness(64);

    uint8_t errors_count = robot.error_status;
    if (errors_count > 7) {
        errors_count = 7;
    }

    if (errors_count != 0) {
        robot.led.fill(mGray);
    }

    for (uint8_t i = 0; i < 7; i++) {
        if (i < errors_count) {
            if (errors_count != 7) {
                robot.led.set(i, mOrange);
            } else {
                robot.led.set(i, mRed);   
            }
            
        }
    }

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
    robot.last_fast_millis = millis();

    robot.tasks_move.push( TaskSent() );
    robot.tasks_move.push(TaskDelay(700));

    LOG_INFO("Robot Link Successful");
}

void loop() {
    auto& robot = Robot::instance();
    robot.timer_slow.update();
    robot.timer_fast.update();
}