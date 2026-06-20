#include "occlusion.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

bool OcclusionMonitor::isocclued() {
    press = sensor_press();
    if (press > thrshold) {
        return false;
    }
    return true;
}