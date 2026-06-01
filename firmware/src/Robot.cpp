#include "Robot.h"

#include "Log.h"

void loop() {
    robot.timer_slow.update();
    robot.timer_fast.update();
}

void Robot::loop_slow() {

}

void Robot::loop_fast() {
    imu.get(ypr);

    update_encoder();
    update_pi();

    if (!tasks.isEmpty()) {
        tasks.top().execute();
    }

    quad.rpm(rpm, steer);
    update_tasks();
}

