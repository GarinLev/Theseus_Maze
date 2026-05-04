#include "node_color.h"

void rgb_to_hsv(uint16_t r, uint16_t g, uint16_t b, float& h, float& s, float& v) {
    float rf = (float)r / 65535.0f;
    float gf = (float)g / 65535.0f;
    float bf = (float)b / 65535.0f;

    float maxVal = max(max(rf, gf), bf);
    float minVal = min(min(rf, gf), bf);
    float delta = maxVal - minVal;

    v = maxVal;
    s = (maxVal > 0.0f) ? (delta / maxVal) : 0.0f;

    if (delta == 0.0f) h = 0.0f;
    else {
        if (maxVal == rf)      h = 60.0f * fmod(((gf - bf) / delta), 6.0f);
        else if (maxVal == gf) h = 60.0f * (((bf - rf) / delta) + 2.0f);
        else if (maxVal == bf) h = 60.0f * (((rf - gf) / delta) + 4.0f);
        if (h < 0) h += 360.0f;
    }
}

void node_color_init(ColorNode& ctx) {
    PT_INIT(&ctx.pt);
    ctx.tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_4X);
    ctx.last_read = 0;
}
int node_color_run(ColorNode& ctx) {
    PT_BEGIN(&ctx.pt);

    for (;;) {
        PT_WAIT_UNTIL(&ctx.pt,
            (ctx.tcs.read8(TCS34725_STATUS) & TCS34725_STATUS_AVALID)
        );

        ctx.c = ctx.tcs.read16(TCS34725_CDATAL);
        ctx.r = ctx.tcs.read16(TCS34725_RDATAL);
        ctx.g = ctx.tcs.read16(TCS34725_GDATAL);
        ctx.b = ctx.tcs.read16(TCS34725_BDATAL);

        rgb_to_hsv(ctx.r, ctx.g, ctx.b, ctx.h, ctx.s, ctx.v);


        PT_YIELD(&ctx.pt);
    }

    PT_END(&ctx.pt);
}