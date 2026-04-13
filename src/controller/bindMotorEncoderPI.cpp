#include "bind.h"
#include "../node/nodes.h"
#include <Arduino.h>
#include <math.h>

static float getVelRPM(int32_t period, float ticks_per_rev) {
    if (period > 0) {
        return (1000000.0f / (float)period) / ticks_per_rev * 60.0f;
    }
    return 0.0f;
}

void bind::MotorEncoderPI(MotorNode *motorNode, EncoderNode *encoderNode, PI_Node *piNode, 
                    float setpoint) {
    
    int32_t current_period = encoderNode->topic->period;
    
    float current_rpm = getVelRPM(current_period, 500.0f);
    
    piNode->setpoint = (int16_t)setpoint;
    piNode->dt = 10;
    
    float error = setpoint - current_rpm;
    if (fabsf(error) > 20.0f) {
        piNode->integral = 0.0f;
    }
    
    node_pi_run(*piNode, &current_rpm);
    
    float base_pwm = (setpoint / 140.0f) * 255.0f;
    
    float output_pwm = base_pwm + piNode->topic->value;
    
    if (output_pwm > 255.0f) output_pwm = 255.0f;
    if (output_pwm < 0.0f) output_pwm = 0.0f;
    
    motorNode->topic->speed = (int)output_pwm;
    
    NOTIFY_TOPIC(motorNode->topic);
    node_motor_run(*motorNode);
}
