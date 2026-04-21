#include <Arduino.h>
#include "node_encoder.h"

void node_encoder_init(EncoderNode &ctx) {
    PT_INIT(&ctx.pt);

    pinMode(ctx.pin_dir, INPUT);
    pinMode(ctx.pin_int, INPUT);

    if (digitalPinToInterrupt(ctx.pin_int) == -1)
        for(;;);
    
    attachInterrupt(digitalPinToInterrupt(ctx.pin_int), ctx.func, RISING);
}