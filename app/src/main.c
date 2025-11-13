/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"


#define SLEEP_MS 1

static const uint8_t PASSWORD[] = {4, 4, 2, 1, 4, 2}; // [1]
#define PASSWORD_SIZE (sizeof(PASSWORD) / sizeof(PASSWORD[0]))

btn_id btns[] = {BTN0, BTN1, BTN2, BTN3};

uint8_t get_state_mask(void) {
  uint8_t mask = 0;

  for (int i = 0; i < 4; i++) {
    if (BTN_check_clear_pressed(btns[i])) mask |= (1 << i);
  }

  return mask;

}

int main(void) {

  if (0 > BTN_init() || 0 > LED_init()) return 0;

  uint8_t in_stream[PASSWORD_SIZE];
  uint8_t i = 0;
  bool lock = true;

  LED_set(LED0, LED_ON);

  while(1) {
    uint8_t state = get_state_mask();

    switch (state) {
      case 0x01: // btn0
      case 0x02: // btn1
      case 0x04: // btn2
        if (!lock) {
          LED_set(LED0, LED_ON);
          lock = true;
        }
        if (i < PASSWORD_SIZE) {
          in_stream[i++] = state;
          for (int j = 0; j < i; j++) printk("%d", in_stream[j]);
          printk("\n");
          break;
        }


      case 0x08: // btn3
        if (!lock) {
            LED_set(LED0, LED_ON);
            lock = true;
          }
        bool correct = true;

        if (i != PASSWORD_SIZE) correct = false;
        else {
          for (int j = 0; j < PASSWORD_SIZE; j++) {
            if (in_stream[j] != PASSWORD[j]) {
              correct = false;
              break;
            }
        }

        }
        

        printk("%s\n", correct ? "Correct!" : "Incorrect!");

        memset(in_stream, 0, sizeof(in_stream));
        i = 0;

        if (correct) {
          LED_set(LED0, LED_OFF);
          lock = false;
        }
        break;

      default:
        break;
    }

    k_msleep(SLEEP_MS);
  }
	return 0;
}

/*
[1]
static const uint8_t PASSWORD[] = {4, 4, 2, 1, 4, 2};
password is a combination of 1, 2, 4 -> 0001, 0010, 0100 in binary.
*/