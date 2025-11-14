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

// GPIO devicetree definitions for buttons
#define SW0_NODE DT_ALIAS(sw0)  // Button 1
#define SW1_NODE DT_ALIAS(sw1)  // Button 2
#define SW2_NODE DT_ALIAS(sw2)  // Button 3
#define SW3_NODE DT_ALIAS(sw3)  // Button 4

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

// Button debouncing
static bool button_was_pressed[4] = {false, false, false, false};

int main(void) {
  int ret;

  // Initialize LEDs
  if (0 > LED_init()) {
    return 0;
  }

  // Check if buttons are ready
  if(!gpio_is_ready_dt(&button0) || !gpio_is_ready_dt(&button1) ||
     !gpio_is_ready_dt(&button2) || !gpio_is_ready_dt(&button3)) {
    printk("Error: buttons not ready\n");
    return -1;
  }

  // Configure buttons as inputs
  ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
  if(ret < 0) return ret;
  
  ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
  if(ret < 0) return ret;

  // Initialize state machine
  state_machine_init();

  // Main loop
  while(1) { 
    // Read all button states with debouncing
    int btn0 = gpio_pin_get_dt(&button0);
    int btn1 = gpio_pin_get_dt(&button1);
    int btn2 = gpio_pin_get_dt(&button2);
    int btn3 = gpio_pin_get_dt(&button3);
    
    // Button 1 (SW0)
    if(btn0 > 0 && !button_was_pressed[0]) {
      state_machine_set_button(BUTTON_1, true);
      button_was_pressed[0] = true;
    } else if(btn0 == 0) {
      button_was_pressed[0] = false;
    }
    
    // Button 2 (SW1)
    if(btn1 > 0 && !button_was_pressed[1]) {
      state_machine_set_button(BUTTON_2, true);
      button_was_pressed[1] = true;
    } else if(btn1 == 0) {
      button_was_pressed[1] = false;
    }
    
    // Button 3 (SW2)
    if(btn2 > 0 && !button_was_pressed[2]) {
      state_machine_set_button(BUTTON_3, true);
      button_was_pressed[2] = true;
    } else if(btn2 == 0) {
      button_was_pressed[2] = false;
    }
    
    // Button 4 (SW3)
    if(btn3 > 0 && !button_was_pressed[3]) {
      state_machine_set_button(BUTTON_4, true);
      button_was_pressed[3] = true;
    } else if(btn3 == 0) {
      button_was_pressed[3] = false;
    }

    // Run state machine
    ret = state_machine_run(); 
    if (ret < 0) {
      return 0;
    }

    k_msleep(SLEEP_MS);
  }
  
  return 0;
}