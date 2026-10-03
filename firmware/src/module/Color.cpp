#include "Color.h"
#include "Robot.h"

const char* COLOR_NAMES[] = { "WHITE", "BLUE", "BLACK", "SILVER", "RED" };

float Color::read_normalized() {
    raw_led = analogRead(pin_led);
    float range = led_max - led_min;
    if (range <= 0.0f) return 0.0f;
    float normalized = ((float)raw_led - led_min) / range;
    return constrain(1.0f - normalized, 0.0f, 1.0f);
}

void Color::init() {
    if(!tcs.begin()) {
        LOG_ERROR("TCS34725 not detected");
        Robot::instance().error_status |= 3;
    }
    pinMode(pin_led, INPUT);
}

void Color::update() {
    uint16_t r_raw = tcs.read16(TCS34725_RDATAL);
    uint16_t g_raw = tcs.read16(TCS34725_GDATAL);
    uint16_t b_raw = tcs.read16(TCS34725_BDATAL);
    c = tcs.read16(TCS34725_CDATAL);

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

    a = read_normalized();
    c_norm = constrain(((float)c - c_min) / (c_max - c_min), 0.0f, 1.0f);

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


float Color::distance_to(const ColorVector& target) const {
    float rad1 = this->h * DEG_TO_RAD;
    float rad2 = target.h * DEG_TO_RAD;

    float x1 = this->s * cosf(rad1);
    float y1 = this->s * sinf(rad1);

    float x2 = target.s * cosf(rad2);
    float y2 = target.s * sinf(rad2);

    float dx = x1 - x2;
    float dy = y1 - y2;
    float da = this->a - target.a;
    float dc = this->c_norm - target.c_norm;

    return sqrtf((dx * dx + dy * dy) + 1.5f * (da * da) + 1.5f * (dc * dc));
}

ColorType Color::get_current_color(float max_distance) const {
    int best_idx = -1;
    float min_dist = 999.0f;

    for (int i = 0; i < 5; ++i) {
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

void Color::dump_calibration_code() {
    Serial.println(F("\n// ====== COPY-PASTE INTO Color.h targets[5] ======"));
    Serial.println(F("ColorVector targets[5] = {"));
    for (int i = 0; i < 5; ++i) {
        Serial.print(F("    { "));
        if (targets[i].h < 100.0f) Serial.print(F(" "));
        if (targets[i].h < 10.0f)  Serial.print(F(" "));
        Serial.print(targets[i].h, 1); Serial.print(F("f, "));
        Serial.print(targets[i].s, 2); Serial.print(F("f, "));
        Serial.print(targets[i].a, 2); Serial.print(F("f, "));
        Serial.print(targets[i].c_norm, 2); Serial.print(F("f }"));
        if (i < 4) Serial.print(F(","));
        else Serial.print(F(" "));
        Serial.print(F(" // "));
        Serial.println(COLOR_NAMES[i]);
    }
    Serial.println(F("};"));
    Serial.println(F("====================================================\n"));
}

void Color::calibrate(ColorType color) {
    if (color >= COLOR_UNKNOWN) return;
    update();
    targets[color] = { h, s, a, c_norm };

    Serial.println();
    Serial.print(F("// Copy for ")); Serial.println(COLOR_NAMES[color]);
    Serial.print(F("{ "));
    Serial.print(h, 1); Serial.print(F("f, "));
    Serial.print(s, 2); Serial.print(F("f, "));
    Serial.print(a, 2); Serial.print(F("f, "));
    Serial.print(c_norm, 2); Serial.print(F("f }, // "));
    Serial.println(COLOR_NAMES[color]);
    Serial.println();
}

void Color::handle_command(const String& cmd) {
    String c_str = cmd;
    c_str.trim();

    if (c_str.equalsIgnoreCase("DUMP")) {
        dump_calibration_code();
        return;
    }

    if (c_str.length() == 1 && c_str[0] >= '0' && c_str[0] <= '4') {
        uint8_t idx = c_str[0] - '0';
        calibrate((ColorType)idx);
        return;
    }

    for (int i = 0; i < 5; ++i) {
        if (c_str.equalsIgnoreCase(COLOR_NAMES[i])) {
            calibrate((ColorType)i);
            return;
        }
    }

    Serial.print(F("Unknown color cmd: '"));
    Serial.print(c_str);
    Serial.println(F("'. Use: C WHITE, C SILVER, C BLUE, C BLACK, C RED, or C DUMP"));
}

void Color::log() {
    update();
    ColorType curr = get_current_color(0.50f);

    Serial.println(F("\n================= COLOR DIAGNOSTICS (4D) ================="));
    Serial.print(F("[RAW] ADC Pin: ")); Serial.print(raw_led);
    Serial.print(F(" | Clear Raw: ")); Serial.print(c);
    Serial.print(F(" | RGB: ("));
    Serial.print(r); Serial.print(F(", "));
    Serial.print(g); Serial.print(F(", "));
    Serial.print(b); Serial.println(F(")"));

    Serial.print(F("[4D COORD] Hue: ")); Serial.print(h, 1);
    Serial.print(F(" | Sat: ")); Serial.print(s, 2);
    Serial.print(F(" | A: ")); Serial.print(a, 2);
    Serial.print(F(" | C_norm: ")); Serial.println(c_norm, 2);

    Serial.print(F("[DIST 4D] "));
    for (uint8_t i = 0; i < 5; ++i) {
        Serial.print(COLOR_NAMES[i]);
        Serial.print(F(": "));
        Serial.print(distance_to(targets[i]), 3);
        if (i < 4) Serial.print(F(" | "));
    }
    Serial.println();

    Serial.print(F("[RESULT] Detected Color -> "));
    Serial.println(curr == COLOR_UNKNOWN ? "UNKNOWN" : COLOR_NAMES[curr]);
    Serial.println(F("==========================================================\n"));
}


void Color::reset_median() {
    median_sum_cos = 0.0f;
    median_sum_sin = 0.0f;
    median_sum_s = 0.0f;
    median_sum_a = 0.0f;
    median_sum_c = 0.0f;
    median_count = 0;
}

void Color::push_median() {
    float rad = h * DEG_TO_RAD;
    median_sum_cos += cosf(rad);
    median_sum_sin += sinf(rad);
    median_sum_s += s;
    median_sum_a += a;
    median_sum_c += c_norm;
    median_count++;
}

ColorVector Color::get_median() const {
    if (median_count == 0) {
        return { h, s, a, c_norm };
    }

    // Круговое среднее для Hue исключает ошибку перехода через 0° / 360° (актуально для красного)
    float avg_cos = median_sum_cos / median_count;
    float avg_sin = median_sum_sin / median_count;
    float avg_h = atan2f(avg_sin, avg_cos) * RAD_TO_DEG;
    if (avg_h < 0.0f) {
        avg_h += 360.0f;
    }

    return {
        avg_h,
        median_sum_s / median_count,
        median_sum_a / median_count,
        median_sum_c / median_count
    };
}

ColorType Color::get_median_color(float max_distance) const {
    if (median_count == 0) {
        return get_current_color(max_distance);
    }

    ColorVector med = get_median();
    int best_idx = -1;
    float min_dist = 999.0f;

    for (int i = 0; i < 5; ++i) {
        float rad1 = med.h * DEG_TO_RAD;
        float rad2 = targets[i].h * DEG_TO_RAD;

        float x1 = med.s * cosf(rad1);
        float y1 = med.s * sinf(rad1);
        float x2 = targets[i].s * cosf(rad2);
        float y2 = targets[i].s * sinf(rad2);

        float dx = x1 - x2;
        float dy = y1 - y2;
        float da = med.a - targets[i].a;
        float dc = med.c_norm - targets[i].c_norm;

        float d = sqrtf((dx * dx + dy * dy) + 1.5f * (da * da) + 1.5f * (dc * dc));
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