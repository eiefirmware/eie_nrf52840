/*
 * main.c

 when on:
  LED0 = locked, time to guess password!
  LED1 = input custom password to guess later
  LED3 = window to set custom pass is open press btn3 then input desired
    password combo, then click btn3 again to set it! then you can input it
    as a proper.. password... guess... check? idk
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"


#define SLEEP_MS 1

// define default password params in case user doesn't input them
static const uint8_t def_password[] = {4, 4, 2, 1, 4, 2}; // [1]
uint8_t def_password_size = (sizeof(def_password) / sizeof(uint8_t));

#define MAX_PASS_SIZE 16


btn_id btns[] = {BTN0, BTN1, BTN2, BTN3};

// gets bitmask of input!!!
uint8_t get_state_mask(void) {
  uint8_t mask = 0;

  for (int i = 0; i < 4; i++) {
    if (BTN_check_clear_pressed(btns[i])) mask |= (1 << i);
  }

  return mask;

}


int main(void) {
  if (0 > BTN_init() || 0 > LED_init()) return 0;

  uint8_t in_stream[MAX_PASS_SIZE];
  uint8_t i = 0;
  uint8_t password[MAX_PASS_SIZE];
  password[0] = 0;



  // Password Set
  int j = 3000;
  LED_set(LED3, LED_ON);
  bool setup = true;

  // j represents ms left, gives user 3 seconds to give a password
  // breaks loop once user has put in a password
  while (0 < j && password[0] == 0) {
    
    // if user hits btn3, it allows them to input their own password
    uint8_t state = get_state_mask();
    if (state == 0x08) {
      LED_set(LED2, LED_ON);
      while (setup && MAX_PASS_SIZE > i) {
        k_msleep(SLEEP_MS);
        state = get_state_mask();

        switch (state) {

          case 0x01: // btn0
          case 0x02: // btn1
          case 0x04: // btn2
            // appends user input to password
            password[i++] = state;
            break;

          case 0x08:
            setup = false;
            j = 0;
            break;

          default: 
            break;

        }
      }
      LED_set(LED2, LED_OFF);
    }
    // times for 3s :)
    k_msleep(SLEEP_MS);
    j--;
  }
  LED_set(LED3, LED_OFF);

  uint8_t password_size;

  if (password[0] == 0) {
    password_size = def_password_size;
    memcpy(password, def_password, def_password_size);
  }
  else 
    password_size = i;

  // prints the password.. probably not great if you... want to guess...
  for (j = 0; j < password_size; j++) printk("%d", password[j]);
  printk("\n");
  


  // Password... guess :)
  i = 0;
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
        if (i < password_size) {
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

        if (i != password_size) correct = false;
        else {
          for (int j = 0; j < password_size; j++) {
            if (in_stream[j] != password[j]) {
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