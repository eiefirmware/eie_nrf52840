/*
 * main.c
 */

#include <zephyr/kernel.h> // includes zephyr kernel functions like k_msleep() for delays and timing
#include <zephyr/drivers/gpio.h> // includes GPIO driver functions to control pins
#include <zephyr/device.h> // includes device driver structures and functions for hardware management
#include <zephyr/sys/printk.h> // includes printk function for debugging to the console
#include <inttypes.h> // includes standard integer types 

// these create macros that reference LED hardware definitions from the device tree
// DT ALIAS looks up the hardware description for LED0 

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define SW0_NODE DT_ALIAS(sw0) // creates a macro that references the button hardware definition from the device tree


// these create GPIO specification structures for each LED, each structure contains which GPIO port the LED is on, which pin number, and configuration flags
// GET extracts this information from the device tree
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);


static struct gpio_callback button_isr_data; // structure hold callback info for the button interrupt its used to register our interrupt handler function with the GPIO driver
static uint8_t counter = 0; // 8bit unsigned integer to hold the counter value 8 meaning (0-255)
static bool button_pressed = false; //boolean flag to indicate if the button was pressed, set to true in the interrupt handler and checked in the main loop

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  button_pressed = true;
}

void update_leds(uint8_t value) {
  // Display the 4-bit value on the LEDs
  gpio_pin_set_dt(&led0, (value & 0x01) ? 1 : 0);  // Bit 0
  gpio_pin_set_dt(&led1, (value & 0x02) ? 1 : 0);  // Bit 1
  gpio_pin_set_dt(&led2, (value & 0x04) ? 1 : 0);  // Bit 2
  gpio_pin_set_dt(&led3, (value & 0x08) ? 1 : 0);  // Bit 3
}

int main(void) {
  int ret;

  // Check if button is ready
  if(!gpio_is_ready_dt(&button0)) {
    printk("Button not ready\n");
    return 0;
  }

  // Configure button as input with interrupt
  ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
  if(ret < 0) {
    printk("Error configuring button\n");
    return 0;
  }

  ret = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
  if(ret < 0) {
    printk("Error configuring interrupt\n");
    return 0;
  }

  gpio_init_callback(&button_isr_data, button_isr, BIT(button0.pin));
  gpio_add_callback(button0.port, &button_isr_data);

  // Configure all LEDs as outputs
  gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

  printk("4-bit binary counter started\n");
  update_leds(counter);

  while(1) {
    if(button_pressed) {
      button_pressed = false;
      
      counter++;
      if(counter >= 16) {
        counter = 0;  // Reset after reaching 16
      }
      
      printk("Counter: %d (Binary: %d%d%d%d)\n", 
             counter,
             (counter & 0x08) ? 1 : 0,
             (counter & 0x04) ? 1 : 0,
             (counter & 0x02) ? 1 : 0,
             (counter & 0x01) ? 1 : 0);
      
      update_leds(counter);
      
      k_msleep(200);  // Debounce delay
    }
    
    k_msleep(10);
  }

  return 0;
}