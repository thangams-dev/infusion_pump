#include <cstdint>
#include <cmath>
#include "volume.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

static constexpr float ms_per_hr = 3600000.0F;
static constexpr float accuracy_lvl = 5.0F;
static constexpr float tot_percentage = 100.0F;
auto VolumeTracker::cal(float rate) -> bool {
    elapsed_time = static_cast<float>(k_uptime_get() - initial) / ms_per_hr;
    expected = elapsed_time * rate;
    ticks    = static_cast<float>(atomic_get(&tick_count));
    actual = ticks * ml_per_tick; 

    if (expected < 0.0001F) { return true; }

    deviation = fabsf(expected - actual) / expected * tot_percentage;

    static uint32_t print_cnt = 0U;
        printk("Ticks:%d|Act:%d|Exp:%d|Dev:%d\n",
               (int)ticks,
               (int)(actual * 1000),
               (int)(expected * 1000),  
               (int)deviation);
    return deviation <= accuracy_lvl;
}