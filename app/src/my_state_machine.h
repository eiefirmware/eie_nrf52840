/**
 * @file my_state_machine.h
 */

#ifndef MY_STATE_MACHINE_H 
#define MY_STATE_MACHINE_H

#include <stdbool.h>

// Button definitions matching your hardware
typedef enum {
    BUTTON_1 = 0,
    BUTTON_2 = 1,
    BUTTON_3 = 2,
    BUTTON_4 = 3
} button_t;

void state_machine_init(void); 
int state_machine_run(void);
void state_machine_set_button(button_t button, bool pressed);

#endif // MY_STATE_MACHINE_H