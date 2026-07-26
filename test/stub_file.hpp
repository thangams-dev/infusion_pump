#pragma once
#include <cstdint>

/// Host-build test doubles: controllable fake state for hardware I/O.
extern int64_t fake_time;
extern uint32_t fake_ticks;
extern float fake_press;
extern bool running;

float sensor_press();
int32_t get_encoder_position();
int64_t k_uptime_get();
void set_delay_rate(uint32_t);
void motor_stop();
void motor_start();
void hardware_init();