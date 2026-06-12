#include "Implementation_file.cpp"
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0),gpios);
static const struct gpio_dt_spec buzzer = GPIO_DT_SPEC_DT(DT_ALIAS(buz0),gpios);
static const struct gpio_dt_spec dir = GPIO_DT_SPEC_GET(DT_ALIAS(dir1),gpios);
static const struct gpio_dt_spec 