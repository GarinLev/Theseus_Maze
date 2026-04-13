#include "robot.h"
#include "../soft_move.h"
SoftMove soft;


void robot::loop() {
    soft.x0 = 0.0f;
    soft.x1 = 100;
    soft.Vmax = 10.0f;
    soft.a = 2.0f;

    soft.startTime = 0;


    float target = SoftGet(&soft);
    static uint32_t last_msg_time = 0;


    apdateVelocity(target);

    if (millis() - last_msg_time >= 100) {
        last_msg_time = millis();

        
        Serial.print(target); Serial.print(",");
        Serial.print(wheelA1.getCurrentRPM()); Serial.print(",");
        Serial.print(wheelA2.getCurrentRPM()); Serial.print(",");
        Serial.print(wheelB1.getCurrentRPM()); Serial.print(",");
        Serial.println(wheelB2.getCurrentRPM());
    }
}
