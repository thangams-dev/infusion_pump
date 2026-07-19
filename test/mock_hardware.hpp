#pragma once
#include <cstdio>
#include <cstdint>
#include <cmath>
#include "pumpmode.hpp"
#define printk(...) printf(__VA_ARGS__)

typedef int atomic_t;
extern bool running;
extern atomic_t tick_count;
extern int64_t fake_time;
extern uint32_t fake_ticks;
extern float fake_press;

float sensor_press();
uint32_t atomic_get(void*);
int64_t k_uptime_get();
void set_delay_rate(uint32_t);
void motor_stop();
void motor_start();
void hardware_init();
void atomic_inc(atomic_t*);
void atomic_set(atomic_t*, int);