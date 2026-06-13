#include "Color.h"
#include "Log.h"

void Color::init() {
    tcs.begin();
}

void Color::update() {
    c = tcs.read16(TCS34725_CDATAL);
    r = tcs.read16(TCS34725_RDATAL);
    g = tcs.read16(TCS34725_GDATAL);
    b = tcs.read16(TCS34725_BDATAL);

    hsv();

    /*Serial.print("H: "); Serial.print(h);
    Serial.print(" S: "); Serial.print(s);
    Serial.print(" V: "); Serial.print(v);
    Serial.print(" R: "); Serial.print(r);
    Serial.print(" G: "); Serial.print(g);
    Serial.print(" B: "); Serial.print(b);
    Serial.print(" C: "); Serial.print(c);
    Serial.print(" S: "); Serial.println(r + g + b);*/


    constexpr HSVColor targets[] = { TARGET_WHITE, TARGET_BLUE, TARGET_BLACK, TARGET_SILVER };
    constexpr size_t colors_count = sizeof(targets) / sizeof(targets[0]);

    compute(targets, last_probabilities, colors_count);
}

ColorType Color::get_current_color(float threshold) const {
    const char* color_names[] = { "WHITE", "BLUE", "BLACK", "SILVER" };

    for (uint8_t i = 0; i < 4; ++i) {
        if (last_probabilities[i] >= threshold) {
            Serial.print(" -> DETECTED: ");
            Serial.print(color_names[i]);
            Serial.print(" (");
            Serial.print(last_probabilities[i]);
            Serial.println("%)");

            return static_cast<ColorType>(i);
        }
    }

    return COLOR_UNKNOWN;
}

void Color::compute(const HSVColor targets[], float outputs[], size_t count) const {
    if (count == 0) return;

    float sum = 0.0f;
    float confidence_factor = 35.0f;

    for (size_t i = 0; i < count; ++i) {
        float dist = match(targets[i]);
        outputs[i] = expf(-dist * confidence_factor);
        sum += outputs[i];
    }

    if (sum > 0.00001f) {
        for (size_t i = 0; i < count; ++i) {
            outputs[i] = (outputs[i] / sum) * 100.0f;
        }
    } else {
        float equal_share = 100.0f / (float)count;
        for (size_t i = 0; i < count; ++i) {
            outputs[i] = equal_share;
        }
    }
}

float Color::match(HSVColor target) const {
    float dh = fabsf(this->h - target.h);
    if (dh > 180.0f) dh = 360.0f - dh;
    float norm_dh = dh / 180.0f;

    float ds = fabsf(this->s - target.s);
    float dv = fabsf(this->v - target.v);

    float dc = fabsf((float)this->c - (float)target.c) / 1600.0f;
    if (dc > 1.0f) dc = 1.0f;

    constexpr float w_h = 0.25f;
    constexpr float w_s = 0.35f;
    constexpr float w_v = 0.15f;
    constexpr float w_c = 0.25f;

    float distance = (w_h * norm_dh) + (w_s * ds) + (w_v * dv) + (w_c * dc);

    if (distance > 1.0f) distance = 1.0f;
    return distance;
}

void Color::calibrate_color() {
    uint16_t rw, gw, bw, cw;
    tcs.getRawData(&rw, &gw, &bw, &cw);

    if (cw > 0) {
        float avg = (float)(rw + gw + bw) / 3.0f;
        Serial.print("Rf: "); Serial.print(avg / (float)rw);
        Serial.print(" Gf: "); Serial.print(avg / (float)gw);
        Serial.print(" Bf: "); Serial.println(avg / (float)bw);
    }
}

void Color::hsv() {
    if (c == 0) {
        h = 0; s = 0; v = 0;
        return;
    }

    auto rf_cal = (float)r * Rf;
    auto gf_cal = (float)g * Gf;
    auto bf_cal = (float)b * Bf;

    float max_val = rf_cal;
    if (gf_cal > max_val) max_val = gf_cal;
    if (bf_cal > max_val) max_val = bf_cal;

    if (max_val == 0.0f) {
        h = 0; s = 0; v = 0;
        return;
    }

    float rf = rf_cal / max_val;
    float gf = gf_cal / max_val;
    float bf = bf_cal / max_val;

    float mx = rf; if (gf > mx) mx = gf; if (bf > mx) mx = bf;
    float mn = rf; if (gf < mn) mn = gf; if (bf < mn) mn = bf;

    float delta = mx - mn;

    v = max_val / 1600.0f;
    if (v > 1.0f) v = 1.0f;

    s = (mx <= 0.0f) ? 0.0f : (delta / mx);

    if (delta < 0.0001f) {
        h = 0.0f;
    } else {
        if (mx == rf) {
            h = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        } else if (mx == gf) {
            h = (bf - rf) / delta + 2.0f;
        } else {
            h = (rf - gf) / delta + 4.0f;
        }
        h *= 60.0f;
    }
}