#include <cmath>
#include "occlusion.hpp"

#ifdef UNIT_TEST
#include "stub_file.hpp"
#include <cstdio>
#define printk printf
#else
#include <zephyr/sys/printk.h>
#endif

/// @brief Checks pressure against threshold, logs status every 2s.
/// @return true if pressure is at/below threshold (no occlusion).
auto OcclusionMonitor::isPressureNormal() -> bool {
    press = sensor_press();

    static int64_t last_print = 0;
    int64_t now_ms = k_uptime_get();
    if (now_ms - last_print >= 2000) {
        printk("[PRESSURE] Current:%d.%02d kPa | Threshold:%d.%02d kPa\n",
               (int)press, (int)(fabsf(press - (int)press) * 100),
               (int)thrshold, (int)(fabsf(thrshold - (int)thrshold) * 100));
        last_print = now_ms;
    }
    return press <= thrshold;
}