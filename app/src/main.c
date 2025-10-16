/*
 * main.c
 */

 /**/




 /*
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;

    Make sure the GPIO device (port) is ready 
    if (!device_is_ready(led0.port)) {
        return -1;
    }

     Configure pin as output and start in the "off" state 
    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return ret;
    }

    while (1) {
        gpio_pin_toggle_dt(&led0);
    }

    return 0;
}
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

int main(void)
{
    while (1) {
        k_msleep(1000); // placeholder; does nothing useful yet
    }
    // (We never return in embedded apps)
    return 0;
}
