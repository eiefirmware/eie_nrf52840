/*
 * main.c
 */

 
// Exercise make the other 3 LED's blink

#include <zephyr/kernel.h> 
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
  
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

// Exercise make the other 3 LED's blink

int main(void) {
  int ret; // declare a variable to store return values
  int counter = 0; // create a counter variable to control which LED's blink and when 
   
  // Check if all LEDs are ready "!" means NOT so if ! is not ready then function returns -1 (error) and program stops
  if(!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) ||
     !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)) {
    return -1;
  }

  // Configure all LEDs as outputs
  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE); // GPIO_OUTPUT_ACTIVE means it starts in the active state 
  if(ret < 0) { // if configuration fails return the error code
    return ret;
  }

  ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  if(ret < 0) {
    return ret;
  }

  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
  if(ret < 0) {
    return ret;
  }

  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
  if(ret < 0) {
    return ret;
  }

  // Blink all LEDs
  while(1) {
    // LED0 toggles every iteration (twice as fast)
    gpio_pin_toggle_dt(&led0);

    // LED1, LED2, LED3 toggle every second iteration (normal speed)
    if(counter % 2 == 0) {
      gpio_pin_toggle_dt(&led1);
      gpio_pin_toggle_dt(&led2);
      gpio_pin_toggle_dt(&led3);
    }
    counter++;
    k_msleep(500);
  
  }
	return 0;
}
