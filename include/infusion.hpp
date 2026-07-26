#pragma once
#include <cstdint>
#include "occlusion.hpp"
#include "volume.hpp"
#include "alarm.hpp"

/// @brief Base class for infusion modes (Template Method pattern).
///        run() always does computeTargetRate() -> applyRate() -> checkAlarm();
///        only computeTargetRate() differs per mode.
class InfusionMode {
    public:
    VolumeTracker &volume;      ///< volume tracker reference
    OcclusionMonitor &occlu;    ///< occlusion monitor reference
    AlarmManager &alarm;        ///< alarm manager reference

    float ml_per_sec = 0.0F;       ///< rate in mL/sec
    float steps_per_sec = 0.0F;    ///< rate in steps/sec
    float current_ml = 0.0F;       ///< mL delivered this run
    bool started_ = false;         ///< run started or not
    bool completed_ = false;       ///< infusion done flag
    int64_t start_ms_ = 0;         ///< start time
    int64_t last_alert_ms_ = 0;    ///< last alarm print time
    uint32_t delay = 0U;           ///< delay per motor step (µs)

    /// @brief Binds mode to shared volume/occlusion/alarm modules.
    InfusionMode(VolumeTracker &vol, OcclusionMonitor &occ, AlarmManager &ala)
        : volume(vol), occlu(occ), alarm(ala) {}

    // destructor
    virtual ~InfusionMode() {} // LCOV_EXCL_LINE

    /// @brief Runs one infusion cycle: compute -> apply -> check.
    void run();

    /// @brief Mode-specific rate calculation.
    virtual float computeTargetRate() = 0;

    /// @brief Applies rate to stepper.
    /// @param rate Target rate, mL/hr.
    void applyRate(float rate);

    /// @brief Checks volume/occlusion and raises alarms if needed.
    /// @param rate Current rate, mL/hr.
    void checkAlarm(float rate);
};

/// @brief Fixed-rate infusion mode.
class ConstantRateMode : public InfusionMode {
    public:
    float setrate;   ///< fixed target rate, mL/hr

    /// @brief Sets a fixed infusion rate.
    ConstantRateMode(float rate, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), setrate(rate) {}

    float computeTargetRate() override;
};

/// @brief Step-wise ramping infusion mode, rate increases until capped at fin.
class LinearRampMode : public InfusionMode {
    public:
    float initial;              ///< starting rate, mL/hr
    float incr;                 ///< rate increase per step
    float fin;                  ///< max rate cap, mL/hr
    float current_lvl;          ///< current rate level
    float tot;                  ///< remaining volume to infuse
    float total_volume;         ///< original total volume (for RESET)
    int64_t last_step_ms_ = 0;  ///< uptime of last rate step-up
    static constexpr int64_t step_interval_ms_ = 60000;  ///< time between steps (ms)

    /// @brief Sets ramp start/increment/cap and total volume.
    LinearRampMode(float st, float in, float fi, float total,
                   VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), initial(st), incr(in), fin(fi),
          current_lvl(st), tot(total), total_volume(total) {}

    float computeTargetRate() override;
};

/// @brief Switches active mode without restart, resetting new mode's timing/volume state.
InfusionMode* switch_mode(InfusionMode* current, InfusionMode* next);