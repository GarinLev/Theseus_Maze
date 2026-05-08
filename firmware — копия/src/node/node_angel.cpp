#include "../../lib/GyverIO/GyverIO.h"
#include "node_angel.h"
#include "../../lib/pt/pt.h"
#include "../../lib/AceSorting/AceSorting.h"

void node_angel_init(AngelNode& ctx) {
    PT_INIT(&ctx.pt);

    ctx.mpu.initialize();
    if (!ctx.mpu.testConnection()) {
        Serial.println(F("MPU6050 connection failed"));
        for (;;);
    }

    if (!ctx.mpu.testConnection()) {
        Serial.println(F("MPURepo -> MPU6050 error connection"));
        for (;;);
    }

    const uint8_t devStatus = ctx.mpu.dmpInitialize();
    if (devStatus != 0 && devStatus != 1) {
        Serial.print(F("MPURepo -> DMP Initialization failed - code: "));
        Serial.println(devStatus);
        for (;;);
    }
    
    
    ctx.mpu.setXAccelOffset(-2242);
    ctx.mpu.setYAccelOffset(-3245);
    ctx.mpu.setZAccelOffset(2878);
    ctx.mpu.setXGyroOffset(-1127);
    ctx.mpu.setYGyroOffset(-39);
    ctx.mpu.setZGyroOffset(-11);


    ctx.mpu.setDMPEnabled(true);

    ctx.mpu.resetFIFO();

    ctx.mpu.getIntStatus();
}


int node_angel_run(AngelNode& ctx) {
    if (ctx.mpu.dmpGetCurrentFIFOPacket(ctx.fifoBuffer)) {
        Quaternion q;
        VectorFloat gravity;

        ctx.mpu.dmpGetQuaternion(&q, ctx.fifoBuffer);
        ctx.mpu.dmpGetGravity(&gravity, &q);
        ctx.mpu.dmpGetYawPitchRoll(ctx.ypr, &q, &gravity);

        ctx.ypr[0] = ctx.ypr[0] * 180 / PI;
        ctx.ypr[1] = ctx.ypr[1] * 180 / PI;
        ctx.ypr[2] = ctx.ypr[2] * 180 / PI;
    }
}