/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


struct gpio_dt_spec{
    /** GPIO device controlling the pin */
    const struct device *port;
    /** The pin's number on the device */
    gpio_pin_t pin;
    /** The pin's configuration flags as specified in the devicetree*/
    gpio_dt_flags_t dt_flags;
};

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void) {
    int ret;

    if(!gpio_is_ready_dt(&led0)) {
        return -1;
    }

    ret = gpio_pin_configure_dt(&ledo0, GPIO_OUTPUT_ACTIVE);
    if(ret < 0){
        return ret;
    }

    while (1) {
    }

    return 0;
}