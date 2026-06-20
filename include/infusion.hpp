#pragma once
#include <stdint.h>
#include "occlusion.hpp"
#include "volume.hpp"
#include "alarm.hpp"
/// @brief it will run the apllyRate and checkAarm for everytime, whenever, we any Mode is called, it have a common computeTargetRate
class InfusionMode{
    public:
    VolumeTracker &volume;
    OcclusionMonitor &occlu;
    AlarmManager &alarm;
    float ml_sec;
    float step_sec;
    bool started_ = false;
    int64_t start_ms_ = 0;

    uint32_t delay;
        InfusionMode(VolumeTracker &vol,OcclusionMonitor &occ, AlarmManager &ala) : volume(vol) , occlu(occ) , alarm(ala){}
        virtual float computeTargetRate() = 0; 
    /// @brief Execute one infusion cycle
    void run();
    /// @brief Apply computed rate to stepper @param rate mL/hr
    void applyRate(float rate);
    /// @brief Check and trigger alarms if needed
    void checkAlarm();
};
/// @brief It have a constant rate speed in Motor
class ConstantRateMode : public InfusionMode{
    public:
    float setrate;
ConstantRateMode(float rate, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
    : InfusionMode(vt, om, am), setrate(rate) {}
float computeTargetRate();
};
/// @brief it have Ramp mode speed in Motor
class LinearRampMode : public InfusionMode{
public:
    float initial;
    float incr;
    float fin;
    float current_lvl;
LinearRampMode(float st, float in, float fi, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
    : InfusionMode(vt, om, am), initial(st), incr(in), fin(fi), current_lvl(st) {}
float computeTargetRate();
};