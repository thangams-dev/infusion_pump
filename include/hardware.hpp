    #pragma once
    #include <stdint.h>
    #include "pumpmode.hpp"
    #ifndef UNIT_TEST
    #include <zephyr/devicetree.h>
    #include <zephyr/drivers/gpio.h>
    #include <zephyr/drivers/uart.h>

    /// @brief Encoder ISR callback
    void encoder_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
    #endif
    /// @brief Initialize all GPIO and peripherals
    void hardware_init();

    /// @brief Read pressure sensor value @return pressure in hPa
    float sensor_press();

    /// @brief Read switch to determine pump mode @return PumpMode enum
    PumpMode get_pump_mode();