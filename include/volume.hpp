#pragma once
#include <cstdint>

/// @brief Tracks infused volume via encoder ticks and checks accuracy against target rate.
class VolumeTracker {
public:
    int64_t initial = 0;              ///< start time of infusion
    float encoder_offset = 0.0F;      ///< encoder count at start
    float ticks = 0;                  ///< ticks moved since start
    float expected = 0.0F;            ///< mL that should be delivered
    float deviation = 0.0F;           ///< % diff, expected vs actual
    float actual = 0.0F;              ///< mL actually delivered
    int64_t last_calc_ms = 0;         ///< last check timestamp
    int64_t start_ms = 0;             ///< set on every START, drives grace period
    int64_t last_print = 0;           ///< last status-print timestamp

    static constexpr float ml_per_rotation    = 0.1F;
    static constexpr float ticks_per_rotation = 2400.0F;
    static constexpr float ml_per_tick = ml_per_rotation / ticks_per_rotation;

    /// @brief Checks delivered volume against expected, for given rate.
    /// @param rate Target rate, mL/hr.
    /// @return true if deviation is within 5%.
    bool cal(float rate);
};