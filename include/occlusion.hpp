#pragma once
#include <stdint.h>
#include "hardware.hpp"
/// @brief Get the pressure and check it with thrshold 
class OcclusionMonitor{
    public:
    float press;   
    static constexpr float thrshold = 300.0f;
    /// @brief check the the press and thrshold and @return true or false
    bool isocclued();
};