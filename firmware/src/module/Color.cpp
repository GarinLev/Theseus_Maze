#include "Color.h"
#include <Arduino.h>

const char* COLOR_NAMES[] = { "WHITE", "BLUE", "BLACK", "SILVER" };

float Color::read_normalized() const {
    uint16_t c_raw = analogRead(pin_led);
    float range = led_max - led_min;
    if (range <= 0.0f) return 0.0f;
    float normalized = ((float)c_raw - led_min) / range;
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    return 1.0f - normalized;
}


void Color::init() {
    tcs.begin();
    pinMode(pin_led, INPUT);
}

void Color::update() {
    uint16_t r_raw = tcs.read16(TCS34725_RDATAL);
    uint16_t g_raw = tcs.read16(TCS34725_GDATAL);
    uint16_t b_raw = tcs.read16(TCS34725_BDATAL);

    float alpha = 0.85f;
    r = (uint16_t)(r * (1.0f - alpha) + r_raw * alpha);
    g = (uint16_t)(g * (1.0f - alpha) + g_raw * alpha);
    b = (uint16_t)(b * (1.0f - alpha) + b_raw * alpha);

    c = read_normalized();

    hsv();

    constexpr HSVColor targets[] = { TARGET_WHITE, TARGET_BLUE, TARGET_BLACK, TARGET_SILVER };
    compute(targets, last_probabilities, 4);
}

void Color::hsv() {
    float rf_cal = (float)r * Rf;
    float gf_cal = (float)g * Gf;
    float bf_cal = (float)b * Bf;

    float max_val = rf_cal;
    if (gf_cal > max_val) max_val = gf_cal;
    if (bf_cal > max_val) max_val = bf_cal;

    if (max_val == 0.0f) { h = s = 0; return; }

    float rf = rf_cal / max_val;
    float gf = gf_cal / max_val;
    float bf = bf_cal / max_val;

    float mx = rf; if (gf > mx) mx = gf; if (bf > mx) mx = bf;
    float mn = rf; if (gf < mn) mn = gf; if (bf < mn) mn = bf;
    float delta = mx - mn;

    s = (mx <= 0.0f) ? 0.0f : (delta / mx);

    if (delta < 0.0001f) {
        h = 0.0f;
    } else {
        if (mx == rf) h = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        else if (mx == gf) h = (bf - rf) / delta + 2.0f;
        else h = (rf - gf) / delta + 4.0f;
        h *= 60.0f;
    }
}

float Color::match(HSVColor target) const {
    float dh = fabsf(this->h - target.h);
    if (dh > 180.0f) dh = 360.0f - dh;
    float norm_dh = dh / 180.0f;

    float ds = fabsf(this->s - target.s);
    float dc = fabsf(this->c - target.c);

    constexpr float S_MAX_TRUST = 0.20f;
    float trust_factor = this->s / S_MAX_TRUST;
    if (trust_factor > 1.0f) trust_factor = 1.0f;
    float w_h = 0.01f + (0.44f * trust_factor);

    if (this->c > 0.90f && target.c > 0.90f) {
        float high_brightness_zone = (this->c - 0.90f) / 0.10f;

        float c_amp = 1.0f + (high_brightness_zone * 24.0f);
        dc *= c_amp;

        w_h *= (1.0f - high_brightness_zone * 0.98f);
    }

    float w_remaining = 1.0f - w_h;
    float w_s = w_remaining * 0.40f;
    float w_c = w_remaining * 0.60f;

    float distance = (w_h * norm_dh) + (w_s * ds) + (w_c * dc);
    return min(distance, 1.0f);
}

void Color::compute(const HSVColor targets[], float outputs[], size_t count) const {
    float sum = 0.0f;
    float conf = 15.0f;

    for (size_t i = 0; i < count; ++i) {
        outputs[i] = expf(-match(targets[i]) * conf);
        sum += outputs[i];
    }
    for (size_t i = 0; i < count; ++i) {
        outputs[i] = (outputs[i] / sum) * 100.0f;
    }
}

ColorType Color::get_current_color(float threshold) const {
    for (uint8_t i = 0; i < 4; ++i) if (last_probabilities[i] >= threshold) return (ColorType)i;
    return COLOR_UNKNOWN;
}

void Color::log() {
    Serial.print("[BALANCING] "); calibrate_color();
    update();
    Serial.print("[CODE] constexpr HSVColor NAME = {");
    Serial.print(h, 2); Serial.print("f, ");
    Serial.print(s, 2); Serial.print("f, ");
    Serial.print(c, 2); Serial.println("f};");
    Serial.print("[PROBABILITIES] ");
    for (uint8_t i = 0; i < 4; ++i) {
        Serial.print(COLOR_NAMES[i]); Serial.print(": ");
        Serial.print(last_probabilities[i], 1); Serial.print("% | ");
    }
    Serial.println();
    ColorType curr = get_current_color(80.0f);
    Serial.print("[RESULT] Color: ");
    Serial.println(curr == COLOR_UNKNOWN ? "UNKNOWN" : COLOR_NAMES[curr]);
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