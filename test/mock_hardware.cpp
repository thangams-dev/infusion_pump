#include "mock_hardware.hpp"
#include "alarm.hpp"
#include "pumpmode.hpp"

atomic_t tick_count = 0;
int64_t fake_time = 0;
uint32_t fake_ticks = 0;
float fake_press = 0.0F;
bool running = false;

float sensor_press() { 
    return fake_press; 
}
uint32_t atomic_get(void*) { 
    return fake_ticks; 
}
int64_t k_uptime_get() { 
    return fake_time; }
void set_delay_rate(uint32_t) {}
void motor_stop() {}
void motor_enable() {}
void hardware_init() {}
void atomic_inc(atomic_t*) {}
void atomic_set(atomic_t* t, int v) { *t = v; fake_ticks = v; }

void led::update() {}
void led::clear() {}
void buz::update() {}
void buz::clear() {}

PumpMode get_pump_mode() { 
    return PumpMode::Constant_mode; 
}