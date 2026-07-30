#ifdef UNIT_TEST
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#else
#include "hardware.hpp"
#include "alarm.hpp"

static const struct gpio_dt_spec led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec buzzer  = GPIO_DT_SPEC_GET(DT_ALIAS(buz0), gpios);
static const struct gpio_dt_spec dir     = GPIO_DT_SPEC_GET(DT_ALIAS(dir1), gpios);
static const struct gpio_dt_spec enab    = GPIO_DT_SPEC_GET(DT_ALIAS(enab1), gpios);
static const struct device *qdec = DEVICE_DT_GET(DT_ALIAS(qdec0));
static const struct pwm_dt_spec step_pwm = PWM_DT_SPEC_GET(DT_ALIAS(step1));
const struct device *lps = DEVICE_DT_GET_ANY(st_lps22hb_press);

/// @brief Initializes all GPIO/PWM/sensor peripherals, checks each is ready.
void hardware_init() {
    if (!device_is_ready(led_pin.port)) { printk("FAIL: led_pin\n"); return; }
    if (!device_is_ready(step_pwm.dev)) { printk("FAIL: pwm\n"); return; }
    if (!device_is_ready(lps))          { printk("FAIL: lps\n"); return; }
    if (!device_is_ready(qdec))         { printk("FAIL: qdec\n"); return; }

    gpio_pin_configure_dt(&buzzer, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&dir, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&enab, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&dir, 0);

    printk("Hardware initialized OK\n");
}

/// @brief Reads LPS22HB pressure sensor.
/// @return Pressure in kPa, or 0.0 if sensor not ready.
float sensor_press() {
    if (!device_is_ready(lps)) {
        return 0.0F;
    }
    struct sensor_value pressure;
    sensor_sample_fetch(lps);
    sensor_channel_get(lps, SENSOR_CHAN_PRESS, &pressure); // returns kPa
    return static_cast<float>(sensor_value_to_double(&pressure));
}

void led::update(bool active) {
    gpio_pin_set_dt(&led_pin, active ? 1 : 0);
}

void buz::update(bool active) {
    gpio_pin_set_dt(&buzzer, active ? 1 : 0);
}

void uart_observer::update(bool active) {
    if (active) {
        printk("ALARM ACTIVE\n");
    }
}

/// @brief Disables the TMC2209 motor driver (EN active-low).
void motor_stop() {
    gpio_pin_set_dt(&enab, 0);  // EN high = disabled
}

/// @brief Enables the TMC2209 motor driver (EN active-low).
void motor_start() {
    gpio_pin_set_dt(&enab, 1);  // EN low = enabled
}

/// @brief Sets stepper pulse rate via PWM.
/// @param delay_us Delay between steps, in microseconds.
void set_delay_rate(uint32_t delay_us) {
    uint32_t period_ns = delay_us * 1000U;
    pwm_set_dt(&step_pwm, period_ns, period_ns / 2U);  // 50% duty cycle
}

/// @brief Reads quadrature encoder and returns cumulative tick count.
int32_t get_encoder_position() {
    struct sensor_value val;
    sensor_sample_fetch(qdec);
    sensor_channel_get(qdec, SENSOR_CHAN_ROTATION, &val);
    int32_t raw = (val.val1 * 2400) / 360;   // 0-2400 within one rotation

    static int32_t last_raw = raw;  // it will run once
    static int32_t cumulative = 0; 

    int32_t delta = raw - last_raw;

    // detect wrap: large jump means it wrapped around
    if (delta < -1200) {
        delta += 2400;   // wrapped forward (e.g. 2350 -> 20)
    } else if (delta > 1200) {
        delta -= 2400;   // wrapped backward (reverse direction)
    }

    cumulative += delta;
    last_raw = raw;

    return cumulative;
}

#endif