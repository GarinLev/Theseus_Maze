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

    constexpr HSVColor targets[] = { TARGET_WHITE, TARGET_BLUE, TARGET_BLACK, TARGET_SILVER };
    constexpr size_t colors_count = sizeof(targets) / sizeof(targets[0]);

    compute(targets, last_probabilities, colors_count);
}

ColorType Color::get_current_color(float threshold) const {
    for (uint8_t i = 0; i < 4; ++i) {
        if (last_probabilities[i] >= threshold) {
            return static_cast<ColorType>(i);
        }
    }
    return COLOR_UNKNOWN;
}

void Color::compute(const HSVColor targets[], float outputs[], size_t count) const {
    if (count == 0) return;

    float sum = 0.0f;
    float confidence_factor = 45.0f;

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
    if (dh > 180.0f) {
        dh = 360.0f - dh;
    }
    float norm_dh = dh / 180.0f;

    float ds = fabsf(this->s - target.s);
    float dv = fabsf(this->v - target.v) * 60.0f;

    constexpr float w_h = 0.40f;
    constexpr float w_s = 0.40f;
    constexpr float w_v = 0.20f;

    float distance = (w_h * norm_dh) + (w_s * ds) + (w_v * dv);

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

    float rf = rf_cal / max_val;
    float gf = gf_cal / max_val;
    float bf = bf_cal / max_val;

    float mx = rf; if (gf > mx) mx = gf; if (bf > mx) mx = bf;
    float mn = rf; if (gf < mn) mn = gf; if (bf < mn) mn = bf;

    float delta = mx - mn;

    v = (float)c / 65535.0f;
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