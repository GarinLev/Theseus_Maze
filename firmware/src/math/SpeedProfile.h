#ifndef FIRMWARE_SPEEDPROFILE_H
#define FIRMWARE_SPEEDPROFILE_H

class SpeedProfile {
public:
    SpeedProfile() = default;

    SpeedProfile(float ss, float su, float l, float lu, float ld)
        : ss(ss), su(su), l(l), lu(lu), ld(ld) {}

    float compute(float ln) const;
    float get_len() const { return l; }
private:
    float ss = 0;
    float su = 0;
    float l = 0;
    float lu = 0;
    float ld = 0;
};

#endif
