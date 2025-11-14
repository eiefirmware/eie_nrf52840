/*
 * main.c
 */
#include <inttypes.h>

#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "my_state_machine.h"

#define SLEEP_MS 1

// Add GPIO devicetree definitions
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define SW0_NODE DT_ALIAS(sw0)
  
// GPIO specifications for all LEDs and button
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

// Function to update LEDs based on counter value (4-bit binary display)
void update_leds(int counter) {
  // Display the 4-bit counter value on the LEDs
  gpio_pin_set_dt(&led0, (counter & 0x01) ? 1 : 0); // Bit 0 (LSB)
  gpio_pin_set_dt(&led1, (counter & 0x02) ? 1 : 0); // Bit 1
  gpio_pin_set_dt(&led2, (counter & 0x04) ? 1 : 0); // Bit 2
  gpio_pin_set_dt(&led3, (counter & 0x08) ? 1 : 0); // Bit 3 (MSB)
}

int main(void) {
  int ret;
  int counter = 0; // 4-bit counter (0-15)
  bool button_was_pressed = false; // Track button state for debouncing

  // Initialize buttons using your existing BTN module
  if (0 > BTN_init()) { 
    return 0;
  }
  
  // Initialize LEDs using your existing LED module
  if (0 > LED_init()) {
    return 0;
  }

  // Additional GPIO setup for direct LED control
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

  // Configure all LEDs as outputs (starting OFF)
  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
  if(ret < 0) return ret;

  // Display initial counter value (0000 in binary)
  update_leds(counter);

  // Initialize your state machine
  state_machine_init(); 

  // Main loop
  while(1) { 
    // Read button state
    int button_state = gpio_pin_get_dt(&button0);
    
    // Button press detection with debouncing
    if(button_state > 0) {
      if(!button_was_pressed) {
        counter++; // Increment counter
        
        // Wrap counter at 16 (back to 0)
        if(counter >= 16) {
          counter = 0;
        }
        
        // Update LEDs to show binary counter value
        update_leds(counter);
        
        button_was_pressed = true;
      }
    } else {
      button_was_pressed = false; // Button released
    }

    // Run your state machine
    ret = state_machine_run(); 
    if (ret < 0) {
      return 0;
    }

    k_msleep(SLEEP_MS); // 1ms sleep
  }
  
  return 0;
}