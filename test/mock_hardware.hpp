// mock_hardware.hpp
#pragma once
#include <cstdint>

typedef int atomic_t;
extern atomic_t tick_count;
extern int64_t fake_time;
extern uint32_t fake_ticks;
extern float fake_press;

float sensor_press();
uint32_t atomic_get(void*);
int64_t k_uptime_get();
void set_delay_rate(uint32_t);