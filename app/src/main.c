/*
 * main.c
 */

#include <zephyr/kernel.h> 
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define SW0_NODE DT_ALIAS(sw0)  // Add button definition
  
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

// Function to update LEDs based on counter value
void update_leds(int counter) {
  // Display the 4-bit counter value on the LEDs
  // Bit 0 (LSB) -> LED0
  gpio_pin_set_dt(&led0, (counter & 0x01) ? 1 : 0);
  // Bit 1 -> LED1
  gpio_pin_set_dt(&led1, (counter & 0x02) ? 1 : 0);
  // Bit 2 -> LED2
  gpio_pin_set_dt(&led2, (counter & 0x04) ? 1 : 0);
  // Bit 3 (MSB) -> LED3
  gpio_pin_set_dt(&led3, (counter & 0x08) ? 1 : 0);
}

int main(void) {
  int ret; // declare a variable to store return values
  int counter = 0; // create a counter variable (0-15 for 4-bit)
  bool button_was_pressed = false; // track button state
   
  // Check if all LEDs are ready
  if(!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) ||
     !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)) {
    return -1;
  }

  // Check if button is ready
  if(!gpio_is_ready_dt(&button0)) {
    return -1;
  }

  // Configure button as input
  ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
  if(ret < 0) {
    return ret;
  }

  // Configure all LEDs as outputs (starting INACTIVE/OFF)
  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) {
    return ret;
  }

  ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) {
    return ret;
  }

  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) {
    return ret;
  }

  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) {
    return ret;
  }

  // Display initial counter value (0000)
  update_leds(counter);

  // Main loop
  while(1) {
    // Read button state
    int button_state = gpio_pin_get_dt(&button0);
    
    // Check if button is currently pressed
    if(button_state > 0) {
      // Only increment if button wasn't already pressed (detect press, not hold)
      if(!button_was_pressed) {
        counter++; // increment counter
        
        // Reset counter when it reaches 16
        if(counter >= 16) {
          counter = 0;
        }
        
        // Update LEDs to show new counter value in binary
        update_leds(counter);
        
        button_was_pressed = true; // mark that button is pressed
      }
    } else {
      // Button released
      button_was_pressed = false;
    }
    
    k_msleep(50); // Small delay for debouncing
  }
  
  return 0;
}