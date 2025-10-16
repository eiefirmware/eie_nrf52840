/*
 * main.c
 */
 
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

int main(void) {
    while (1) {
        k_msleep(10000);
    }

    return 0;
}
