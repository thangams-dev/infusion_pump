#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include "pumpmode.hpp"
#include "Implementation_file.hpp"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0),gpios);
static const struct gpio_dt_spec buzzer = GPIO_DT_SPEC_GET(DT_ALIAS(buz0),gpios);
static const struct gpio_dt_spec dir = GPIO_DT_SPEC_GET(DT_ALIAS(dir1),gpios);
static const struct gpio_dt_spec step = GPIO_DT_SPEC_GET(DT_ALIAS(step1),gpios);
static const struct gpio_dt_spec enc = GPIO_DT_SPEC_GET(DT_ALIAS(enc1),gpios);
static const struct gpio_dt_spec sw = GPIO_DT_SPEC_GET(DT_ALIAS(sw1),gpios);
const struct device *lps = DEVICE_DT_GET_ANY(st_lps22hb_press);
static const struct gpio_dt_spec enab = GPIO_DT_SPEC_GET(DT_ALIAS(enab1),gpios);
static struct gpio_callback cb_data;
struct sensor_value pressure;
atomic_t tick_count = ATOMIC_INIT(0);
void encoder_isr(const struct device* dev, struct gpio_callback*cb, uint32_t pins){
    atomic_inc(&tick_count);
}

void hardware_init(){
gpio_pin_configure_dt(&led,GPIO_OUTPUT_INACTIVE);
gpio_pin_configure_dt(&buzzer,GPIO_OUTPUT_INACTIVE);
gpio_pin_configure_dt(&dir,GPIO_OUTPUT_INACTIVE);
gpio_pin_configure_dt(&step,GPIO_OUTPUT_INACTIVE);
gpio_pin_configure_dt(&enc, GPIO_INPUT|GPIO_PULL_UP);
gpio_pin_configure_dt(&enab, GPIO_INPUT|GPIO_PULL_UP);
gpio_pin_configure_dt(&sw,GPIO_INPUT| GPIO_PULL_UP);
gpio_pin_interrupt_configure_dt(&enc, GPIO_INT_EDGE_TO_ACTIVE);
gpio_init_callback(&cb_data,encoder_isr,BIT(enc.pin));
gpio_add_callback(enc.port,&cb_data);
}
float sensor_press(){   
    sensor_sample_fetch(lps);
    sensor_channel_get(lps, SENSOR_CHAN_PRESS, &pressure);
    return sensor_value_to_double(&pressure);
}

pumpMode get_pump_mode(){
    if(gpio_pin_get_dt(&sw)){
        return pumpMode::Ramp_mode;
    }
    return pumpMode::Constant_mode;
}
void led::update(){
    gpio_pin_set_dt(&led,1);
}
void buz::update(){
    gpio_pin_set_dt(&buzzer,1);
}


