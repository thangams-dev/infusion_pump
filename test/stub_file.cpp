#include "alarm.hpp"
#include "alarm_observer.hpp"
#include "stub_file.hpp"

int64_t fake_time  = 0;
uint32_t fake_ticks = 0;
float fake_press = 0.0F;
bool running = false;

float sensor_press() {
    return fake_press;
}

int32_t get_encoder_position() {
    return static_cast<int32_t>(fake_ticks);
}

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