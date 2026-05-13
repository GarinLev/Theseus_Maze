#include "controller_color.h"
#include "../robot/robot.h"

void ColorController::init() {
    PT_INIT(&pt_task);

    tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

    if (tcs.begin()) {
        last_update_ms = millis();
    }
    else {
        Serial.println("Error: TCS34725 not found!");
    }
}


int ColorController::update() {
    PT_BEGIN(&pt_task);

    for (;;) {
        c = tcs.read16(TCS34725_CDATAL);
        r = tcs.read16(TCS34725_RDATAL);
        g = tcs.read16(TCS34725_GDATAL);
        b = tcs.read16(TCS34725_BDATAL);

        rgb_to_hsv(r, g, b, c, h, s, v);

        if (s < 0.1 && !isBlack())
            h = 0;

        Serial.print(isBlue()); Serial.print(" ");
        Serial.print(isBlack()); Serial.print(" ");
        Serial.print(isGrey()); Serial.print(" | ");
        
        Serial.print(h); Serial.print(" ");
        Serial.print(s); Serial.print(" ");
        Serial.print(v); Serial.print(" ");
        Serial.println(c);


        last_update_ms = millis();
        PT_WAIT_UNTIL(&pt_task, millis() - last_update_ms >= 120);
    }

    PT_END(&pt_task);
}


void ColorController::calibrateWhite() {
    uint16_t rw, gw, bw, cw;
    tcs.getRawData(&rw, &gw, &bw, &cw);
    
    if (cw > 0) {
        float avg = (float)(rw + gw + bw) / 3.0f;
        
        Serial.print("Rf: "); Serial.print(avg / (float)rw);
        Serial.print(" Gf: "); Serial.print(avg / (float)gw);
        Serial.print(" Bf: "); Serial.println(avg / (float)bw);

    }
}


void ColorController::calibrateColor() {
    uint16_t r, g, b, c;
    float h, s, v;
    tcs.getRawData(&r, &g, &b, &c);
    rgb_to_hsv(r, g, b, c, h, s, v);

    Serial.print("H: "); Serial.print(h);
    Serial.print(" S: "); Serial.print(s * 100);
    Serial.print(" V: "); Serial.print(v * 100);
    Serial.print(" C: "); Serial.println(c);
}

bool ColorController::isBlack() {
    return (c < robot::OFFSETS_COLOR[4] * 1.2);
}

bool ColorController::isGrey() {
    return (c >= robot::OFFSETS_COLOR[5] * 0.96);
}

bool ColorController::isBlue() {
    return (h != 0) && (h < robot::OFFSETS_COLOR[3] + 20)
        && (h > robot::OFFSETS_COLOR[4] - 20);
}

void ColorController::rgb_to_hsv(uint16_t r, uint16_t g, uint16_t b, uint16_t c,
    float& h, float& s, float& v) {
    
    if (c == 0) {
        h = 0; s = 0; v = 0;
        return;
    }

    float rf_cal = (float)r * robot::OFFSETS_COLOR[0];
    float gf_cal = (float)g * robot::OFFSETS_COLOR[1];
    float bf_cal = (float)b * robot::OFFSETS_COLOR[2];

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