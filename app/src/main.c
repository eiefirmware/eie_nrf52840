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
  if (BTN_init() != 0) {
    return 0;
  }

  int waiting = 1;
  int pass[] = {0, 0, 0};
  while (1) {
    if (waiting) {
      LED_set(LED3, 1);
      if (BTN_check_clear_pressed(BTN3)) {
        waiting = 0;
      }
    } else {
      LED_set(LED3, 0);
      if (BTN_check_clear_pressed(BTN3)) {
        waiting = 1;
        break;
      } else if (BTN_check_clear_pressed()) {
        
      }

    }
    k_msleep(SLEEP_MS);
  }
  return 0;
}
