#include <Arduino.h>
#include "controller_light.h"

void LightController::init()
{
    strip.setBrightness(255);
    strip.clear();
    strip.show();
    last_color = (COLORS)0;
    timer = millis();
}

void LightController::update()
{
    uint32_t now = millis();
    if (now - timer >= 25) {
        timer = now;
        if (color != last_color) {
            last_color = color;
            strip.fill(color);
            strip.show();
        }
    }
}