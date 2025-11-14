/*
 * main.c
 */

#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include "LED.h"
#include "BTN.h"
#include "password_state_machine.h"

#define SLEEP_TIME_MS 50

// Button GPIO definitions
#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

// Button debouncing
static bool btn0_was_pressed = false;
static bool btn1_was_pressed = false;
static bool btn2_was_pressed = false;
static bool btn3_was_pressed = false;

int main(void) {
    int ret;

    // Initialize LED and BTN drivers
    if (0 > LED_init()) {
        return 0;
    }
    if (0 > BTN_init()) {
        return 0;
    }

    // Check if buttons are ready
    if(!gpio_is_ready_dt(&button0) || !gpio_is_ready_dt(&button1) ||
       !gpio_is_ready_dt(&button2) || !gpio_is_ready_dt(&button3)) {
        printk("Error: Buttons not ready\n");
        return -1;
    }

    // Configure all buttons as inputs
    ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
    if(ret < 0) return ret;

    ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
    if(ret < 0) return ret;

    ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
    if(ret < 0) return ret;

    ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
    if(ret < 0) return ret;

    // PWM brightness control variable
    uint8_t current_duty_cycle = 0;
    
    // Set initial LED brightness
    LED_pwm(LED0, current_duty_cycle);

    // Initialize password state machine
    password_state_machine_init();

    // Main loop
    while(1) {
        // Read button states with debouncing
        int btn0_state = gpio_pin_get_dt(&button0);
        int btn1_state = gpio_pin_get_dt(&button1);
        int btn2_state = gpio_pin_get_dt(&button2);
        int btn3_state = gpio_pin_get_dt(&button3);

        // Button 0 (BTN1 on board) - PWM Brightness Control
        if(btn0_state > 0 && !btn0_was_pressed) {
            // Increment duty cycle by 10%, wrap at 100%
            current_duty_cycle = (current_duty_cycle >= 100) ? 0 : (current_duty_cycle + 10);
            printk("Setting LED0 to %d%% brightness.\n", current_duty_cycle);
            LED_pwm(LED0, current_duty_cycle);
            btn0_was_pressed = true;
        } else if(btn0_state == 0) {
            btn0_was_pressed = false;
        }

        // Button 1 (BTN2 on board) - Password input
        if(btn1_state > 0 && !btn1_was_pressed) {
            password_state_machine_button_press(PASSWORD_BUTTON_1); 
            btn1_was_pressed = true;
        } else if(btn1_state == 0) {
            btn1_was_pressed = false;
        }

        // Button 2 (BTN3 on board) - Password input
        if(btn2_state > 0 && !btn2_was_pressed) {
            password_state_machine_button_press(PASSWORD_BUTTON_2);  
            btn2_was_pressed = true;
        } else if(btn2_state == 0) {
            btn2_was_pressed = false;
        }

        // Button 3 (BTN4 on board) - Password Enter button
        if(btn3_state > 0 && !btn3_was_pressed) {
            password_state_machine_button_press(PASSWORD_BUTTON_3); 
            btn3_was_pressed = true;
        } else if(btn3_state == 0) {
            btn3_was_pressed = false;
        }

        // Run state machine
        ret = password_state_machine_run();  
        if(ret < 0) {
            printk("State machine error: %d\n", ret);
            return ret;
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}