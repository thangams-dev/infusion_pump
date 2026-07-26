#pragma once
#include <cstdint>
#include "hardware.hpp"

/// @brief Monitors line pressure against a fixed threshold to detect occlusion.
class OcclusionMonitor {
    public:
    float press;   ///< last read pressure

    static constexpr float thrshold = 100.56F;  ///< occlusion threshold

    /// @brief Checks current pressure against threshold.
    /// @return true if pressure is at/below threshold (no occlusion).
    bool isPressureNormal();
};