/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LEDO_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LEDO_NODE, gpios);

int main(void) {
  int ret;
  
  if(!gpio_is_ready_dt(&led0)) {
    return -1;
  }

  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
  if(ret < 0) {
    return ret;
  }

  while(1) {
    gpio_pin_toggle_dt(&led0);
  
  }
	return 0;
}
