        #pragma once
        #include <stdint.h>
        #ifndef UNIT_TEST
        #include <zephyr/drivers/pwm.h>
        #include <zephyr/devicetree.h>
        #include <zephyr/drivers/gpio.h>
        #include <zephyr/drivers/uart.h>
        #include <zephyr/drivers/sensor.h>
        extern atomic_t tick_count;
        
        /// @brief Encoder ISR callback
        void encoder_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
        #endif
        /// @brief Initialize all GPIO and peripherals
        void hardware_init();

        /// @brief to stop the motor
        void motor_stop();

        void motor_enable();
        
        /// @brief Read pressure sensor value @return pressure in hPa
        float sensor_press();
        /// @brief set the delay per step
        /// @param delay_us delay will depents upon the rate
        void set_delay_rate(uint32_t delay_us);
 