/** * @file main.c */ 
#include <inttypes.h> 
#include <zephyr/kernel.h> 
#include <zephyr/sys/printk.h> 


#include "BTN.h" 
#include "LED.h" 

#define SLEEP_MS 1

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if(0 > LED_init()){
    return 0;
  }

  while(1) {

  }

  }

  void password_input(void) {

    int password[];
    int end = 1;

    for(int i = 0; end == 1 ;i++){
      if (BTN_check_clear_pressed(BTN0)){
        password[i] = 0;
        i++;
    }
      if(BTN_check_clear_pressed(BTN1)){
        password[i] = 1;
        i++;
      }
      if(BTN_check_clear_pressed(BTN2)){
        password[i] = 2;
        i++;
      }
  }
}

  





