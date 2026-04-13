#pragma once

#include "../node/nodes.h"

namespace bind {
    void MotorEncoderPI(MotorNode *motorNode, EncoderNode *encoderNode, PI_Node *piNode, 
                        float setpoint);
};