#include <stdint.h>
#include <cmath>
#include "volume.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

static constexpr float ms_per_hr = 3600000.0f;
static constexpr float accuracy_lvl = 5.0f;
static constexpr float expect = 0.0f;
static constexpr float tot_percentage = 100.0f;
bool VolumeTracker::cal() {
    elapsed_time = (k_uptime_get() - initial) / ms_per_hr;
    ticks = static_cast<uint32_t>(atomic_get(&tick_count));
    expected = elapsed_time * rate;
    actual = ticks * ml;
    if (expected == expect) return true;
    deviation = fabsf(expected - actual) / expected * 100.0f;
    if (deviation > accuracy_lvl) {
        return false;
    }
    return true;
}