#pragma once
#include <stdint.h>
#include "occlusion.hpp"
#include "volume.hpp"
#include "alarm.hpp"

/// @brief Base class for infusion modes. Template Method pattern:
///        run() always calls computeTargetRate() -> applyRate() -> checkAlarm(),
///        but computeTargetRate() is overridden per mode (constant vs ramp).
class InfusionMode {
    public:
    //referance members to alias existing objects
    VolumeTracker &volume;      
    OcclusionMonitor &occlu;    
    AlarmManager &alarm;      

    float ml_per_sec = 0.0F;       // current rate converted to mL/sec
    float steps_per_sec = 0.0F;    // current rate converted to motor steps/sec
    float current_ml = 0.0F;       // volume delivered so far, this run
    bool started_ = false;         // true once start_ms_ has been captured
    bool completed_ = false;       // flags infusion-complete; consumed by main.cpp under lock
    int64_t start_ms_ = 0;         // use to note start time
    int64_t last_alert_ms_ = 0;    // last time a volume alert was printed
    uint32_t delay = 0U;           // to store per-step delay (µs) sent to the stepper driver

    /// @brief constructor for Infusion pump to get object referances
    /// @param vol volume module 
    /// @param occ occulsion module
    /// @param ala alarm 
    InfusionMode(VolumeTracker &vol, OcclusionMonitor &occ, AlarmManager &ala)
        : volume(vol), occlu(occ), alarm(ala) {}

    // no-heap policy forbids delete on base ptr -> deleting-dtor variant unreachable
    // LCOV_EXCL_LINE tells lcov to skip this line from coverage count (not a real gap)
    virtual ~InfusionMode() {} // LCOV_EXCL_LINE

    /// @brief Mode-specific rate calculation - must be implemented by each mode
    virtual float computeTargetRate() = 0;

    /// @brief Execute one infusion cycle (compute -> apply -> check)
    void run();

    /// @brief Apply computed rate to stepper @param rate mL/hr
    void applyRate(float rate);

    /// @brief Check and trigger alarms if needed
    void checkAlarm(float rate);
};

/// @brief Constant rate mode - motor runs at one fixed rate for the whole infusion
class ConstantRateMode : public InfusionMode {
    public:
    float setrate;   // fixed target rate, mL/hr

    ConstantRateMode(float rate, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), setrate(rate) {}

    float computeTargetRate() override;
};

/// @brief Linear ramp mode - rate increases step-wise every interval, up to a max, then holds
class LinearRampMode : public InfusionMode {
    public:
    float initial;         // starting rate, mL/hr
    float incr;             // rate increase per step
    float fin;               // max rate cap, mL/hr
    float current_lvl;      // current rate level (mutates as ramp progresses)
    float tot;               // remaining volume to infuse
    float total_volume;      // original total volume (for RESET)
    int64_t last_step_ms_ = 0;   // uptime (ms) of the last rate step-up
    static constexpr int64_t step_interval_ms_ = 60000;  // time between rate increases (60s)

    LinearRampMode(float st, float in, float fi, float total,
                   VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), initial(st), incr(in), fin(fi),
          current_lvl(st), tot(total), total_volume(total) {}

    float computeTargetRate() override;
};

/// @brief Switch active mode without requiring a restart; resets new mode's timing/volume state
InfusionMode* switch_mode(InfusionMode* current, InfusionMode* next);