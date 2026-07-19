#pragma once
#include <stdint.h>
#ifndef UNIT_TEST
#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>

#include "alarm_observer.hpp"
extern atomic_t tick_count;

/// @brief Quadrature encoder ISR — shared callback for both A and B channel
///        interrupts. Reads current A/B state, decodes direction via lookup
///        table, updates tick_count (+1 forward, -1 reverse, 0 = rejected noise).
void encoder_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
#endif

/// @brief Initialize all GPIO and peripherals
void hardware_init();

/// @brief to stop the motor
void motor_stop();

/// @brief enable the motor driver
void motor_start();

/// @brief Read pressure sensor value @return pressure in hPa
float sensor_press();

/// @brief set the delay per step
/// @param delay_us delay will depents upon the rate
void set_delay_rate(uint32_t delay_us);

/// @brief It get the encoder position
/// @return It return the Count
int32_t get_encoder_position();