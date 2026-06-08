#ifndef FIRMWARE_QUAD_H
#define FIRMWARE_QUAD_H

#include "Wheel.h"

constexpr float Quad_Radius = 35.0f;
constexpr float Quad_CPR = 500.0f;

constexpr float Quad_MM(float mm) {
    return (mm / (2.0f * 3.14 * Quad_Radius)) * Quad_CPR;
}

class Quad {
public:
    Quad() = default;

    Quad(Wheel *fr, Wheel *fl, Wheel *br, Wheel *bl)
        : fr(fr), fl(fl), br(br), bl(bl) {}

    void update(float fr_target, float fl_target, float br_target, float bl_target) const;
    void rpm(float rpm, float steer) const;
    float encoder() const;
    void encoder_reset() const;

private:
    Wheel *fr = nullptr;
    Wheel *fl = nullptr;
    Wheel *br = nullptr;
    Wheel *bl = nullptr;
};

#endif