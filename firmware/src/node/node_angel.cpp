#include "../../lib/GyverIO/GyverIO.h"
#include "node_angel.h"
#include "../../lib/pt/pt.h"
#include "../../lib/AceSorting/AceSorting.h"

#define MAX_VALID_RANGE 230

void node_angel_init(AngelNode& ctx) {
    PT_INIT(&ctx.pt);

    ctx.mpu.initialize();
    if (!ctx.mpu.testConnection()) {
        Serial.println(F("MPU6050 connection failed"));
        for (;;);
    }

    ctx.mpu.setXAccelOffset(0); ctx.mpu.setYAccelOffset(0); ctx.mpu.setZAccelOffset(0);
    ctx.mpu.setXGyroOffset(0); ctx.mpu.setYGyroOffset(0); ctx.mpu.setZGyroOffset(0);

    ctx.mpu.CalibrateAccel(6);
    ctx.mpu.CalibrateGyro(6);
    ctx.mpu.PrintActiveOffsets();

    uint8_t devStatus = ctx.mpu.dmpInitialize();

    if (devStatus == 0) {
        ctx.mpu.setDMPEnabled(true);
    }
    else {
        Serial.print(F("DMP Initialization failed. Code: "));
        Serial.println(devStatus);
        for (;;);
    }
}



int node_angel_run(AngelNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt, (uint32_t)(millis() - ctx.last_time) >= (uint32_t)ctx.dt);
        ctx.last_time = millis();

        Quaternion q;
        VectorFloat gravity;

        ctx.mpu.dmpGetQuaternion(&q, ctx.fifoBuffer);
        ctx.mpu.dmpGetGravity(&gravity, &q);
        ctx.mpu.dmpGetYawPitchRoll(ctx.ypr, &q, &gravity);

        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}
