/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

int main(void) {
  
  uint8_t counter = 0;

  while(1){
    if(BTN_check_clear_pressed(BTN0)){
      counter++;
      if(counter == 16){
        counter = 0;
      }
      LED_set(LED0,(counter & 0x01) ? LED_ON : LED_OFF);
      LED_set(LED1,(counter & 0x02) ? LED_ON : LED_OFF);
      LED_set(LED2,(counter & 0x04) ? LED_ON : LED_OFF);
      LED_set(LED3,(counter & 0x08) ? LED_ON : LED_OFF);
      
      printk("Counter: %d\n", counter);
    }
    k_msleep(10);
  }
  return 0;
}
