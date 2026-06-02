#include "Color.h"

#include "Log.h"

void Color::init() {
    if(tcs.begin()) {
        last_update_ms = millis();
    } else {
        LOG_ERROR("Initialization failed.");
    }
}

void Color::update() {
    c = tcs.read16(TCS34725_CDATAL);
    r = tcs.read16(TCS34725_RDATAL);
    g = tcs.read16(TCS34725_GDATAL);
    b = tcs.read16(TCS34725_BDATAL);

    hsv();
}

void Color::calibrate_color() {
    tcs.getRawData(&r, &g, &b, &c);
    hsv();

    LOG_INFO("H: ", h, " S: ", s, " V: ", v, " C: ", c);

    Serial.print("H: "); Serial.print(h);
    Serial.print(" S: "); Serial.print(s * 100);
    Serial.print(" V: "); Serial.print(v * 100);
    Serial.print(" C: "); Serial.println(c);
}

void Color::hsv() {

    if (c == 0) {
        h = 0; s = 0; v = 0;
        return;
    }

    float rf_cal = (float)r;
    float gf_cal = (float)g;
    float bf_cal = (float)b;

    float max_val = rf_cal;
    if (gf_cal > max_val) max_val = gf_cal;
    if (bf_cal > max_val) max_val = bf_cal;

    float rf = rf_cal / max_val;
    float gf = gf_cal / max_val;
    float bf = bf_cal / max_val;

    float mx = rf;
    if (gf > mx) mx = gf;
    if (bf > mx) mx = bf;

    float mn = rf;
    if (gf < mn) mn = gf;
    if (bf < mn) mn = bf;

    float delta = mx - mn;

    v = (float)c / 65535.0f;
    s = (mx <= 0.0f) ? 0.0f : (delta / mx);

    if (delta < 0.0001f) {
        h = 0.0f;
    }
    else {
        if (mx == rf) {
            h = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        }
        else if (mx == gf) {
            h = (bf - rf) / delta + 2.0f;
        }
        else {
            h = (rf - gf) / delta + 4.0f;
        }
        h *= 60.0f;
    }
}