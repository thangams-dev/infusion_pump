#include "infusion.hpp"
#include <stdlib.h>
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

extern bool running;

static constexpr float sec_per_hr    = 3600.0F;
static constexpr float steps_per_ml  = 1600.0F;
static constexpr float usec_per_sec  = 1000000.0F;
static constexpr float ms_per_tick   = 10.0F;
static constexpr float ms_per_hr     = 3600000.0F;

void InfusionMode::run() {
    float rate = computeTargetRate();
    applyRate(rate);
    checkAlarm(rate);
}

void InfusionMode::applyRate(float rate) {
    if (!started_) {
        start_ms_ = k_uptime_get();
        started_  = true;
    }
    if (rate < 0.001F) { return; }  // guard zero rate
    ml_per_sec    = rate / sec_per_hr;
    steps_per_sec = ml_per_sec * steps_per_ml;
    current_ml    = rate * (ms_per_tick / ms_per_hr);
    motor_enable(); 
    delay         = static_cast<uint32_t>(usec_per_sec / steps_per_sec);
    set_delay_rate(delay);
}

void InfusionMode::checkAlarm(float rate) {
    if (!volume.cal(rate)) {
        printk("Vlome Alert\n");
        alarm.notify();
    }
        static uint32_t print_cnt = 0U;
    if (!occlu.isocclued()) {
        printk("Pressure Alert\n");
        alarm.notify();
    } else {
        alarm.clearAll();
    }
}

auto ConstantRateMode::computeTargetRate() -> float {
    return setrate;
}

auto LinearRampMode::computeTargetRate() -> float {
    int64_t now = k_uptime_get();
    if ((now - last_step_ms_) >= step_interval_ms_) {
        current_lvl += incr;
        if (current_lvl > fin) { current_lvl = fin; }
        last_step_ms_ = now;
    }

    float infused = current_lvl * (ms_per_tick / ms_per_hr);
    tot -= infused;
    if (tot <= 0.0F) {
        running = false;
        motor_stop();
        printk(">> Infusion completed\n");
    }
    return current_lvl;
}