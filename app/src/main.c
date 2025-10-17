/*
 * main.c
 */

#include <zephyr/kernel.h>    //includes functions like k_msleep

#include <BTN.h>
#include <LED.h>

#define SLEEP_TIME_MS 200 /*delay known as software debouncing, 
                            sometimes the signal when clicking will bounce, causing
                            the button to 'flicker', so adding a delay will prevent
                            the system to see a double click and will check it once 
                            before sleeping and then retries
                            
                            test for ideal*/

int main(void) 
{
  int i;
  
  if (BTN_init() < 0) {
    return 0;
  }

  if (LED_init() < 0) {
    return 0;
  }

  while (1) {

    for (i = 0; i < 16; i++) {
      if (BTN_is_pressed(BTN0)) {
        if (i % 2 == 1) {
          LED_toggle(LED3);
          printk("D column of truth table \n");
        }

        // if ( i == 2 || i == 3 || i == 6 || i == 7 || i == 10 || i == 11 || i == 14 || i == 15) {
        //   LED_toggle(LED3);
        //   printk("C column of truth table \n");
        // }
      }
      k_msleep(SLEEP_TIME_MS);
    }
    
  }
  return 0;
}