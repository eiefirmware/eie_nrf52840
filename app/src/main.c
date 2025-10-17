/*
 * main.c
 */

#include <zephyr/kernel.h>    //includes functions like k_msleep
#include <zephyr/device.h>    //includes device struct in gpio_dt_spec
#include <zephyr/drivers/gpio.h>  //includes all gpio_* functions and types
#include <zephyr/sys/printk.h>    //includes printk function
#include <inttypes.h>             //includes fixed sized int like uint8_t and sint32_t

#define SLEEP_TIME_MS 1000

#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct gpio_callback button_isr_data;

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Button 0 pressed!\n");
}

int main(void) {
  int ret;

  if(!gpio_is_ready_dt(&button)) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (0 > ret) {
    return 0;
  }

  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
  if (0 > ret) {
    return 0;
  }  

  gpio_init_callback(&button_isr_data, button_isr, BIT(button.pin));
  gpio_add_callback(button.port, &button_isr_data);

  while(1) {

  }
	return 0;
}
