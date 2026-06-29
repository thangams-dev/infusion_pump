#ifdef UNIT_TEST
#include "mock_hardware.hpp"
#else
#include "hardware.hpp"
#include "alarm.hpp"
static const struct gpio_dt_spec led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(led0),gpios);
static const struct gpio_dt_spec buzzer = GPIO_DT_SPEC_GET(DT_ALIAS(buz0),gpios);
static const struct gpio_dt_spec dir = GPIO_DT_SPEC_GET(DT_ALIAS(dir1),gpios);
static const struct gpio_dt_spec enc = GPIO_DT_SPEC_GET(DT_ALIAS(enc1),gpios);
static const struct gpio_dt_spec sw = GPIO_DT_SPEC_GET(DT_ALIAS(sw1),gpios);
const struct device *lps = DEVICE_DT_GET_ANY(st_lps22hb_press);
static const struct gpio_dt_spec enab = GPIO_DT_SPEC_GET(DT_ALIAS(enab1),gpios);
static const struct pwm_dt_spec step_pwm = PWM_DT_SPEC_GET(DT_ALIAS(step1));
static struct gpio_callback cb_data;
struct sensor_value pressure;
atomic_t tick_count = ATOMIC_INIT(0);
void encoder_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    static int64_t last_tick_ms = 0;
    int64_t now = k_uptime_get();
    if ((now - last_tick_ms) >= 2) {  // 2ms debounce
        atomic_inc(&tick_count);
        last_tick_ms = now;
    }
}

void hardware_init(){
    if (!device_is_ready(led_pin.port)) { return; }
    if (!device_is_ready(buzzer.port)) { return; }
    if (!device_is_ready(dir.port)) { return; }
    if (!device_is_ready(enc.port)) { return; }
    if (!device_is_ready(enab.port)) { return; }
    if (!device_is_ready(sw.port)) { return; }
    if (!device_is_ready(step_pwm.dev)) { return; }

    gpio_pin_configure_dt(&buzzer, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&dir, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&enc, GPIO_INPUT | GPIO_PULL_UP);

gpio_pin_configure_dt(&enab, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&led_pin, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&sw, GPIO_INPUT | GPIO_PULL_UP);

    gpio_pin_interrupt_configure_dt(&enc, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&cb_data, encoder_isr, BIT(enc.pin));

    gpio_add_callback(enc.port, &cb_data);

    gpio_pin_set_dt(&dir, 0);  // set direction forward
}
float sensor_press(){   
        if (!device_is_ready(lps)) {
        return 0.0F;
    }
    sensor_sample_fetch(lps);
    sensor_channel_get(lps, SENSOR_CHAN_PRESS, &pressure);
    return static_cast<float>(sensor_value_to_double(&pressure));
}

void led::update(){
    gpio_pin_set_dt(&led_pin,1);
}
void buz::update(){
    gpio_pin_set_dt(&buzzer,1);
}
void buz::clear(){
    gpio_pin_set_dt(&buzzer, 0);
}
void motor_stop() {
     gpio_pin_set_dt(&enab, 0);  // EN high = TMC2209 disabled
}
void led::clear(){
    gpio_pin_set_dt(&led_pin, 0);
}
void set_delay_rate(uint32_t delay_us) {
    uint32_t period_ns = delay_us * 2000U;  // convert µs to ns
    pwm_set_dt(&step_pwm, period_ns, period_ns / 2U);
}
void motor_enable() {
    gpio_pin_set_dt(&enab, 1);  
}

#endif