#pragma once
#include <stdint.h>

/// @brief Tracks infused volume via encoder ticks
class VolumeTracker {
public:
    uint32_t ticks;
    uint64_t initial;
    float elapsed_time;
    float expected;
    float rate;
    float deviation;
    float ml;
    float actual;

    /// @brief Constructor @param setrate mL/hr @param start_ms start time @param ml_step mL per step
    VolumeTracker(float setrate, uint64_t start_ms, float ml_step)
        : rate(setrate), initial(start_ms), ml(ml_step) {}
    
    /// @brief Calculate volume deviation @return true if within 5%
    bool cal();
};