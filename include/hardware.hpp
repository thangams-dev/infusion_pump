#pragma once
#include <cstdint>
#ifndef UNIT_TEST
#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>

#include "alarm_observer.hpp"

#endif

/// @brief Initializes all GPIO and peripherals.
void hardware_init();

/// @brief Enables the motor driver.
void motor_start();

/// @brief Stops the motor.
void motor_stop();

/// @brief Reads pressure sensor value.
/// @return Pressure in kPa.
float sensor_press();

/// @brief Sets stepper delay per step.
/// @param delay_us Delay in microseconds, derived from target rate.
void set_delay_rate(uint32_t delay_us);

/// @brief Gets current encoder position.
/// @return Cumulative encoder tick count.
int32_t get_encoder_position();