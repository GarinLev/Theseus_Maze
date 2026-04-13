#include "../../../lib/GyverIO/GyverIO.h"
#include "node_motor.h"

#define SPEED_MAX 255
#define SPEED_MIN_SOFT 70
#define SPEED_MIN 0

void node_motor_init(MotorNode &ctx) {
    PT_INIT(&ctx.pt);

    gio::mode(ctx.pin_in1, OUTPUT);
    gio::mode(ctx.pin_in2, OUTPUT);

    gio::write(ctx.pin_in1, 0); 
    gio::write(ctx.pin_in2, 0); 

    SUBSCRIBE_TOPIC(ctx, ctx.topic);
}


int node_motor_run(MotorNode &ctx)
{
    PT_BEGIN(&ctx.pt);
    (void)PT_YIELD_FLAG;
    
    LOOP_FOREVER {
        WAIT_NEW_DATA(ctx);

        int16_t raw_speed = ctx.topic->speed;

        if (raw_speed == 0) {
            gio::write(ctx.pin_in1, LOW);
            gio::write(ctx.pin_in2, LOW);
        } else {
            bool dir = (raw_speed > 0) ^ ctx.reverse;
            uint16_t pwm_value = constrain(abs(raw_speed), SPEED_MIN, SPEED_MAX);
            
            if (pwm_value < SPEED_MIN_SOFT)
                pwm_value = 0;
            
            if (dir) {
                analogWrite(ctx.pin_in1, pwm_value);
                gio::write(ctx.pin_in2, LOW);
            } else {
                analogWrite(ctx.pin_in2, pwm_value);
                gio::write(ctx.pin_in1, LOW);
            }
        }
        
        SYNC_TOPIC(ctx, (*ctx.topic));
    }
    
    PT_END(&ctx.pt);
}