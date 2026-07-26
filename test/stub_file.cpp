#include "alarm.hpp"
#include "alarm_observer.hpp"
#include "stub_file.hpp"

/// Host-build test doubles: replace hardware I/O with controllable fakes.
int64_t fake_time   = 0;
uint32_t fake_ticks  = 0;
float fake_press     = 0.0F;
bool running          = false;

/// @brief Returns fake_press instead of reading a real sensor.
float sensor_press() {
    return fake_press;
}

/// @brief Returns fake_ticks instead of reading a real encoder.
int32_t get_encoder_position() {
    return static_cast<int32_t>(fake_ticks);
}

/// @brief Returns fake_time instead of real uptime.
int64_t k_uptime_get() {
    return fake_time;
}

void set_delay_rate(uint32_t) {}
void motor_stop() {}
void motor_start() {}
void hardware_init() {}

void led::update(bool active) { (void)active; }
void buz::update(bool active) { (void)active; }
void uart_observer::update(bool active) { (void)active; }