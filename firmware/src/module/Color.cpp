#include "Color.h"
#include <Arduino.h>

const char* COLOR_NAMES[] = { "WHITE", "BLUE", "BLACK", "SILVER" };

float Color::read_normalized() {
    raw_led = analogRead(pin_led);
    
    float range = led_max - led_min;
    if (range <= 0.0f) return 0.0f;
    
    float normalized = ((float)raw_led - led_min) / range;
    return constrain(1.0f - normalized, 0.0f, 1.0f);
}

void Color::init() {
    tcs.begin();
    pinMode(pin_led, INPUT);
}

void Color::update() {
    uint16_t r_raw, g_raw, b_raw, c_raw;

  r_raw = tcs.read16(TCS34725_RDATAL);
  g_raw = tcs.read16(TCS34725_GDATAL);
  b_raw = tcs.read16(TCS34725_BDATAL);

    if (r == 0 && g == 0 && b == 0) {
        r = r_raw;
        g = g_raw;
        b = b_raw;
    } else {
        float alpha = 0.70f;
        r = (uint16_t)(r * (1.0f - alpha) + r_raw * alpha);
        g = (uint16_t)(g * (1.0f - alpha) + g_raw * alpha);
        b = (uint16_t)(b * (1.0f - alpha) + b_raw * alpha);
    }

    c = read_normalized();
    hsv();
}

void Color::hsv() {
    float rf = (float)r * Rf;
    float gf = (float)g * Gf;
    float bf = (float)b * Bf;

    float sum = rf + gf + bf;
    if (sum > 0.001f) {
        rf /= sum;
        gf /= sum;
        bf /= sum;
    }

    float mx = rf; if (gf > mx) mx = gf; if (bf > mx) mx = bf;
    float mn = rf; if (gf < mn) mn = gf; if (bf < mn) mn = bf;
    float delta = mx - mn;

    s = (mx <= 0.0001f) ? 0.0f : (delta / mx);

    if (delta < 0.0001f) {
        h = 0.0f;
    } else {
        if (mx == rf)      h = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        else if (mx == gf) h = (bf - rf) / delta + 2.0f;
        else               h = (rf - gf) / delta + 4.0f;
        h *= 60.0f;
        if (h < 0.0f)   h += 360.0f;
        if (h >= 360.0f) h -= 360.0f;
    }
}

float Color::distance_to(HSVColor target) const {
    if (this->c > 0.50f && target.c > 0.50f && this->s < 0.35f && target.s < 0.35f) {
        float ds = (this->s - target.s) * 2.5f;
        float dc = (this->c - target.c);
        return sqrtf(ds * ds + dc * dc);
    }

    float rad1 = this->h * (PI / 180.0f);
    float rad2 = target.h * (PI / 180.0f);

    float x1 = this->s * cosf(rad1);
    float y1 = this->s * sinf(rad1);
    float z1 = this->c;

    float x2 = target.s * cosf(rad2);
    float y2 = target.s * sinf(rad2);
    float z2 = target.c;

    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}
ColorType Color::get_current_color(float max_distance) const {
    constexpr HSVColor targets[] = { TARGET_WHITE, TARGET_BLUE, TARGET_BLACK, TARGET_SILVER };
    
    int best_idx = -1;
    float min_dist = 999.0f;

    for (int i = 0; i < 4; ++i) {
        float d = distance_to(targets[i]);
        if (d < min_dist) {
            min_dist = d;
            best_idx = i;
        }
    }

    if (min_dist > max_distance) {
        return COLOR_UNKNOWN;
    }

    return (ColorType)best_idx;

}

void Color::log() {
    update();
    Serial.print("[RAW_ANALOG] Pin ");
    Serial.print(pin_led);
    Serial.print(": ");
    Serial.print(raw_led);
    Serial.print(" | RGB: ");
    Serial.print(r); Serial.print(", ");
    Serial.print(g); Serial.print(", ");
    Serial.println(b);

    Serial.print(h, 2); Serial.print(" ");
    Serial.print(s, 2); Serial.print(" ");
    Serial.print(c, 2); Serial.println(" ");

    constexpr HSVColor targets[] = { TARGET_WHITE, TARGET_BLUE, TARGET_BLACK, TARGET_SILVER };
    Serial.print("[DISTANCES] ");
    for (uint8_t i = 0; i < 4; ++i) {
        Serial.print(COLOR_NAMES[i]); Serial.print(": ");
        Serial.print(distance_to(targets[i]), 3); Serial.print(" | ");
    }
    Serial.println();


    ColorType curr = get_current_color(0.40f);
    Serial.print("[RESULT] Color: ");
    Serial.println(curr == COLOR_UNKNOWN ? "UNKNOWN" : COLOR_NAMES[curr]);
}