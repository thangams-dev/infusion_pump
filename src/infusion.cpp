#include "infusion.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

static constexpr float sec_per_hr = 3600.0f;
static constexpr float steps_per_ml = 200.0f;
static constexpr float usec_per_sec = 1000000.0f;
    void InfusionMode::run() {
        float rate = computeTargetRate();
        applyRate(rate);
        checkAlarm();
    }
    void InfusionMode::applyRate(float rate) {
        if (!started_) {
            start_ms_ = k_uptime_get();
            started_ = true;
        }
        ml_sec = rate / sec_per_hr;
        step_sec = ml_sec * steps_per_ml;
        delay = static_cast<uint32_t>(usec_per_sec / step_sec);
        set_delay_rate(delay);
    }

    void InfusionMode::checkAlarm() {
        if (!volume.cal()) {
            alarm.notify();
        }
        if (!occlu.isocclued()) {
            alarm.notify();
        }
    }

    float ConstantRateMode::computeTargetRate() {
        return setrate;
    }

    float LinearRampMode::computeTargetRate() {
        current_lvl += incr;
        if (current_lvl > fin) {
            current_lvl = fin;
        }
        return current_lvl;
    }