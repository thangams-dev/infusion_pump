#include <cstdint>
#include <cmath>
#include "volume.hpp"
#include "infusion.hpp"
#ifdef UNIT_TEST
#include "stub_file.hpp"
#include <cstdio>
#define printk printf
#else
#include "hardware.hpp"
#include <zephyr/sys/printk.h>
#endif


static constexpr float ms_per_hr = 3600000.0F;
static constexpr float accuracy_lvl = 5.0F;
static constexpr float tot_percentage = 100.0F;

auto VolumeTracker::cal(float rate, float steps_per_sec) -> bool {
    (void)steps_per_sec;
    ticks = fabsf(static_cast<float>(get_encoder_position()) - encoder_offset);
    
    int64_t now_ms = k_uptime_get();
    if (last_calc_ms == 0) { last_calc_ms = now_ms; }
    float dt_hr = static_cast<float>(now_ms - last_calc_ms) / ms_per_hr;
    expected_accumulated += rate * dt_hr;
    last_calc_ms = now_ms;

    expected = expected_accumulated;
    actual   = ticks * ml_per_tick;

    if (expected < 0.0001F) { return true; }
    if ((now_ms - initial) < 5000) { return true; }

    deviation = fabsf(expected - actual) / expected * tot_percentage;

    static int64_t last_print = 0;
    if (now_ms - last_print >= 2000) {   // reuse now_ms, don't redeclare
        printk("[STATUS] Rate:%d mL/hr | Delivered:%d.%02d mL | Expected:%d.%02d mL | Dev:%d%%\n",
               (int)rate,
               (int)actual, (int)(fabsf(actual - (int)actual) * 100),
               (int)expected, (int)(fabsf(expected - (int)expected) * 100),
               (int)deviation);
        last_print = now_ms;
    }
    return deviation <= (accuracy_lvl + 0.01F);
}