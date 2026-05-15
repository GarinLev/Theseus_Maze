#include "controller_color.h"
#include "../robot/robot.h"

#define CFG_R_GAIN   0
#define CFG_G_GAIN   1
#define CFG_B_GAIN   2
#define CFG_BLUE_H   3
#define CFG_BLACK_C  4
#define CFG_GREY_C   5

void ColorController::init() {
    PT_INIT(&pt_task);
    tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    if (tcs.begin()) {
        last_update_ms = millis();
    }
    else {
        Serial.println(F("Error: TCS34725 not found!"));
    }
}

int ColorController::update() {
    PT_BEGIN(&pt_task);

    for (;;) {
        tcs.getRawData(&r, &g, &b, &c);

        rgb_to_hsv(r, g, b, c, h, s, v);

        if (s < 0.15f && !isBlack()) {
            h = 0;
        }

        Serial.print(isBlue()); Serial.print(" ");
        Serial.print(isBlack()); Serial.print(" ");
        Serial.print(isGrey()); Serial.print(" | ");
        Serial.print(h); Serial.print(" ");
        Serial.print(s); Serial.print(" ");
        Serial.println(v);

        last_update_ms = millis();
        PT_WAIT_UNTIL(&pt_task, millis() - last_update_ms >= 120);
    }

    PT_END(&pt_task);
}

void ColorController::rgb_to_hsv(uint16_t r_raw, uint16_t g_raw, uint16_t b_raw, uint16_t c_raw,
    float& h_out, float& s_out, float& v_out) {
    if (c_raw == 0) {
        h_out = s_out = v_out = 0;
        return;
    }

    float r_f = (float)r_raw * robot::OFFSETS_COLOR[CFG_R_GAIN];
    float g_f = (float)g_raw * robot::OFFSETS_COLOR[CFG_G_GAIN];
    float b_f = (float)b_raw * robot::OFFSETS_COLOR[CFG_B_GAIN];

    float mx = max(r_f, max(g_f, b_f));
    float mn = min(r_f, min(g_f, b_f));
    float delta = mx - mn;

    v_out = mx / 1024.0f;
    if (v_out > 1.0f) v_out = 1.0f;

    s_out = (mx <= 0.0f) ? 0.0f : (delta / mx);

    if (delta < 0.001f) {
        h_out = 0;
    }
    else {
        if (mx == r_f) {
            h_out = (g_f - b_f) / delta + (g_f < b_f ? 6.0f : 0.0f);
        }
        else if (mx == g_f) {
            h_out = (b_f - r_f) / delta + 2.0f;
        }
        else {
            h_out = (r_f - g_f) / delta + 4.0f;
        }
        h_out *= 60.0f;
    }
}

bool ColorController::isBlack() {
    return (c < robot::OFFSETS_COLOR[CFG_BLACK_C] * 1.2f) && (v < 0.15f);
}

bool ColorController::isGrey() {
    return (c > robot::OFFSETS_COLOR[CFG_GREY_C] * 0.92f);
}

bool ColorController::isBlue() {
    if (isBlack() || isGrey()) return false;

    float target = robot::OFFSETS_COLOR[CFG_BLUE_H];
    float diff = fabsf(h - target);
    if (diff > 180.0f) diff = 360.0f - diff;

    return (diff < 25.0f) && (s > 0.1f);
}

void ColorController::calibrateWhite() {
    uint16_t rw, gw, bw, cw;
    tcs.getRawData(&rw, &gw, &bw, &cw);
    if (cw > 0) {
        float avg = (float)(rw + gw + bw) / 3.0f;
        Serial.print(F("R_Gain: ")); Serial.print(avg / (float)rw);
        Serial.print(F(" G_Gain: ")); Serial.print(avg / (float)gw);
        Serial.print(F(" B_Gain: ")); Serial.println(avg / (float)bw);
    }
}

void ColorController::calibrateColor() {
    uint16_t r_r, g_r, b_r, c_r;
    float ch, cs, cv;
    tcs.getRawData(&r_r, &g_r, &b_r, &c_r);
    rgb_to_hsv(r_r, g_r, b_r, c_r, ch, cs, cv);

    Serial.print(F("H: ")); Serial.print(ch);
    Serial.print(F(" S: ")); Serial.print(cs, 2);
    Serial.print(F(" V: ")); Serial.print(cv, 2);
    Serial.print(F(" C: ")); Serial.println(c_r);
}