#include <cstdint>
typedef int atomic_t;
atomic_t tick_count = 0;
int64_t fake_time = 0;
uint32_t fake_ticks = 0;
float fake_press = 0.0f;
float sensor_press(){
    return fake_press;
}
uint32_t atomic_get(void*){
    return fake_ticks;
}
int64_t k_uptime_get(){
    return fake_time;
}
void set_delay_rate(uint32_t) {}
