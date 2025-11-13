/*
 * main.c
 */

#include <zephyr/kernel.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1


int main(void) {
  if (0 > BTN_init()) return 0;
  if (0 > LED_init()) return 0;

  uint8_t count = 0;

  while(1) {    
    if (BTN_check_clear_pressed(BTN0)) {
      count = (count+1) & 0x0F; // [1]
      printk("Counter: %d\n", count);
      for (int i = 0; i < 4; i++) {
        if (count & (1 << i)) LED_set(i, LED_ON); // [2]
        else LED_set(i, LED_OFF);
      }
    }
    k_msleep(SLEEP_MS);
  }
	return 0;
}

/* 
[1]
count = (count+1) & 0x0F;
0x0F == 00001111 (15), so once count == 00010000 (which would be 16),
count results to 00000000 because of bitwise & operator :).

[2]
if (count & (1 << i)) LED_set(i, LED_ON);
checks that count has a 1 at the 'i'th index of its binary
representation. the index looks like xxxx3210, for loop 0<i<4.
*/
