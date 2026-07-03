#pragma once
#include <stdint.h>

/// @brief Tracks infused volume via encoder ticks
class VolumeTracker {
public:
    int64_t initial = 0;
    float correction = 1.0F;
    bool corrected = false;
    float ticks        = 0.0F;
    float elapsed_time = 0.0F;
    float expected     = 0.0F;
    float deviation    = 0.0F;
    float actual       = 0.0F;
    static constexpr float ml_per_tick = 0.00193F;
    

    /// @brief Calculate volume deviation @param rate ml/hr @return true if within 5%
    bool cal(float rate,float steps_per_sec);
};