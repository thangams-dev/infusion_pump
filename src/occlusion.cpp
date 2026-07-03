#include "occlusion.hpp"
#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#endif

auto OcclusionMonitor::isocclued() -> bool {
    press = sensor_press();
  // printk("occlue: %d\n",(int)press);
    
    return press <= thrshold;  // true = no occlusion, false = occluded
}