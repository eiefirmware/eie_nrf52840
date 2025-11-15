/*
 * main.c - Tamagotchi Virtual Pet
 */

#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "LED.h"
#include "BTN.h"
#include "tamagotchi.h"

#define SLEEP_TIME_MS 1

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
static bool btn_was_pressed[4] = {false, false, false, false};

int main(void) {
    int ret;

    // Initialize LED and BTN drivers
    if (0 > LED_init()) {
        printk("ERROR: LED initialization failed\n");
        return -1;
    }
    if (0 > BTN_init()) {
        printk("ERROR: BTN initialization failed\n");
        return -1;
    }

    // Check if buttons are ready
    if(!gpio_is_ready_dt(&button0) || !gpio_is_ready_dt(&button1) ||
       !gpio_is_ready_dt(&button2) || !gpio_is_ready_dt(&button3)) {
        printk("ERROR: Buttons not ready\n");
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

    // Initialize Tamagotchi
    tamagotchi_init();

    printk("🐣 Tamagotchi ready! Starting care routine...\n\n");

    // Main loop
    while(1) {
        // Read button states
        int btn_states[4] = {
            gpio_pin_get_dt(&button0),
            gpio_pin_get_dt(&button1),
            gpio_pin_get_dt(&button2),
            gpio_pin_get_dt(&button3)
        };

        // Check each button with debouncing
        for (int i = 0; i < 4; i++) {
            if (btn_states[i] > 0 && !btn_was_pressed[i]) {
                // Button pressed
                tamagotchi_button_press((tamagotchi_button_t)i);
                btn_was_pressed[i] = true;
            } else if (btn_states[i] == 0 && btn_was_pressed[i]) {
                // Button released
                tamagotchi_button_release((tamagotchi_button_t)i);
                btn_was_pressed[i] = false;
            }
        }

        // Run Tamagotchi state machine
        ret = tamagotchi_run();
        if (ret < 0) {
            printk("ERROR: Tamagotchi error: %d\n", ret);
            return ret;
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}