/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 100

int main(void) {
  if (LED_init() != 0) {
    return 0;
  }


  while (1) {
    k_msleep(SLEEP_MS);

    LED_toggle(LED0);

  }
  return 0;
}
