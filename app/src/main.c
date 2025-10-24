
/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>
/**
 * #include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
 */

#include "BTN.h"
#include "LED.h"

void led_blink(led_id led_n, size_t sleep_time) {
    LED_set(led_n,1);
    k_msleep(sleep_time);
    LED_set(led_n,0);
    k_msleep(sleep_time);
}


int8_t first_one(int8_t const *arr,size_t sz) {
    for(int8_t i = 0; sz > i; i++) {
        if(arr[i] > 0) {
            printk("\n");
            return i;
        }
    }
    return -1;
}

// Important: Assumes pass is a 4 element array of type uint8_t.
void password(int8_t *pass) {
    
    printk("Entering password sequence...\n");
    int8_t but_arr[] = {0,0,0,0};
    int8_t f1=-1;
    bool error = false;
    for(int i = 0; 4 > i; i++) {
        while(1) {
            printk("array: ");
            for(int j = 0; 4 > j; j++) {
                but_arr[j] = BTN_check_clear_pressed(j);
                printk("%d, ",(int)but_arr[j]);
            }
            printk("\n");
            f1 = first_one(but_arr,4);
            printk("f1 val: %d\n",(int)f1);
            if(0 <= f1) {
                led_blink(0,200);
                if(f1 == pass[i]) {
                    k_msleep(500);
                    printk("CORRECT\n");
                    break;
                } else {
                    k_msleep(500);
                    printk("WRONG?\n");
                    error = true;
                    break;
                }
            }
            k_msleep(500);
        }
    }
    if(false == error) {
        led_blink(0,40);
        led_blink(1,40);
        led_blink(2,40);
        led_blink(3,40);
    } else {
        led_blink(0,100);
        led_blink(0,100);
        led_blink(0,100);
        led_blink(0,100);
    }
}


int main(void) {
    int8_t pass[] = {1,1,2,2};
    if(0 > BTN_init()) {
        return 0;
    }
    if(0 > LED_init()) {
        return 0;
    }
    
    while(1) {
        password(pass);
        k_msleep(2000);
    }
    return 0;
}
