#include <cstdint>
#include <cmath>
#include "volume.hpp"
#include "infusion.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

static constexpr float ms_per_hr = 3600000.0F;
static constexpr float accuracy_lvl = 5.0F;
static constexpr float tot_percentage = 100.0F;

auto VolumeTracker::cal(float rate, float steps_per_sec) -> bool {
    (void)steps_per_sec;

    elapsed_time = static_cast<float>(k_uptime_get() - initial) / ms_per_hr;
    expected = elapsed_time * rate;
    ticks    = static_cast<float>(atomic_get(&tick_count));
    actual   = ticks * ml_per_tick * correction;

    if (expected < 0.0001F) { return true; }
    if ((k_uptime_get() - initial) < 5000) { return true; }

    if (!corrected && ticks > 50.0F) {
        correction = expected / (ticks * ml_per_tick);
        corrected  = true;
    }

    deviation = fabsf(expected - actual) / expected * tot_percentage;

    static float last_ticks = 0.0F;
    static int64_t last_t = 0;
    int64_t now_ms = k_uptime_get();
    if (now_ms - last_t >= 4000) {
        printk("rate:%d | Tickes:%d actual:%d (expected:%d) | dev:%d\n",
               (int)rate, (int)ticks, (int)(actual*1000), (int)(expected*1000), (int)deviation);
        last_ticks = ticks;
        last_t = now_ms;
    }
    return deviation <= (accuracy_lvl + 0.01F);
}