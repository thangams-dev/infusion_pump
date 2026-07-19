#pragma once
#include <stdint.h>

/// @brief Tracks infused volume via encoder ticks
class VolumeTracker {
public:
    int64_t initial = 0;
    float correction = 1.0F;
    bool corrected = false;
    float encoder_offset = 0.0F;
    float ticks        = 0.0F;
    float elapsed_time = 0.0F;
    float expected     = 0.0F;
    float deviation    = 0.0F;
    float actual       = 0.0F;
    float expected_accumulated = 0.0F;
    int64_t last_calc_ms = 0;
    static constexpr float ml_per_rotation    = 0.1F;
    static constexpr float ticks_per_rotation = 2400.0F;
    static constexpr float ml_per_tick = ml_per_rotation / ticks_per_rotation;

    /// @brief Calculate volume deviation @param rate ml/hr @return true if within 5%
    bool cal(float rate, float steps_per_sec);
};