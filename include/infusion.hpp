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
        float ml_per_sec = 0.0F;
        float steps_per_sec = 0.0F;
        float current_ml = 0.0F; 
        bool started_ = false;
        int64_t start_ms_ = 0;
        uint32_t delay = 0U;

            InfusionMode(VolumeTracker &vol,OcclusionMonitor &occ, AlarmManager &ala) : volume(vol) , occlu(occ) , alarm(ala){}
            virtual float computeTargetRate() = 0; 
        /// @brief Execute one infusion cycle
        void run();
        /// @brief Apply compu  ted rate to stepper @param rate mL/hr
        void applyRate(float rate);
        /// @brief Check and trigger alarms if needed
        void checkAlarm(float rate);
    };
    /// @brief It have a constant rate speed in Motor
    class ConstantRateMode : public InfusionMode{
        public:
        float setrate;
    ConstantRateMode(float rate, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), setrate(rate) {}
    float computeTargetRate()override;
    };
    /// @brief it have Ramp mode speed in Motor
    class LinearRampMode : public InfusionMode{
    public:
        float initial;
        float incr;
        float fin;
        float current_lvl;
        float tot;
        float total_volume; 
        int64_t last_step_ms_ = 0;
        static constexpr int64_t step_interval_ms_ = 90000;
        LinearRampMode(float st, float in, float fi, float total,
                   VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
        : InfusionMode(vt, om, am), initial(st), incr(in), fin(fi),
          current_lvl(st), tot(total), total_volume(total) {}
    float computeTargetRate()override;
    };
    /// @brief Switch active mode without requiring a restart; resets new mode's timing/volume state
    InfusionMode* switch_mode(InfusionMode* current, InfusionMode* next);