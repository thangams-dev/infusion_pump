#include "infusion.hpp"
#include <stdlib.h>
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

extern bool running;

static constexpr float sec_per_hr   = 3600.0F;
static constexpr float steps_per_ml = 1600.0F;
static constexpr float usec_per_sec = 1000000.0F;
static constexpr float ms_per_tick  = 10.0F;
static constexpr float ms_per_hr    = 3600000.0F;

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
    if (rate < 0.001F) { return; }
    ml_per_sec    = rate / sec_per_hr;
    steps_per_sec = ml_per_sec * steps_per_ml;
    current_ml = rate * ((k_uptime_get() - start_ms_) / ms_per_hr);
    motor_enable();
    delay = static_cast<uint32_t>(usec_per_sec / steps_per_sec);
    set_delay_rate(delay);
}

void InfusionMode::checkAlarm(float rate) {
    static int64_t last_alert = 0;
    if (!volume.cal(rate,steps_per_sec)) {
        int64_t now = k_uptime_get();
        if (now - last_alert >= 2500) {
            printk("Vlome Alert\n");
            last_alert = now;
        }
        alarm.notify();
    }
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
InfusionMode* switch_mode(InfusionMode* current, InfusionMode* next) {
    if (current == next) { return current; }
    motor_stop();
    next->volume.correction = 1.0F;
    next->volume.corrected  = false;
    next->started_ = false;
    next->volume.initial = k_uptime_get();
    atomic_set(&tick_count, 0);
    return next;
}