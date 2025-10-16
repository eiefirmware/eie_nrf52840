/*
 * main.c
 */
/*
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

int main(void) {
    while (1) {
        k_msleep(10000);
    }

    return 0;
}
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;

    if (!device_is_ready(led0.port)) {
        return -1;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return ret;
    }

    while (1) {
        gpio_pin_toggle_dt(&led0);
        k_msleep(10000);  // Wait 10 seconds between toggles
    }

    return 0;
}
