/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LEDO_NODE DT_ALIAS(led0)

int main(void) {

  if(!pgio_is_ready_dt(&led0)) {
    return -1;
  }

  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
  if(ret < 0) {
    return ret;
  }
  
  while(1) {
  
  }
	return 0;
}
