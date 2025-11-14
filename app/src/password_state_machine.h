/**
 * @file password_state_machine.h
 */

#ifndef PASSWORD_STATE_MACHINE_H
#define PASSWORD_STATE_MACHINE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PASSWORD_BUTTON_0 = 0,
    PASSWORD_BUTTON_1 = 1,
    PASSWORD_BUTTON_2 = 2,
    PASSWORD_BUTTON_3 = 3  // Enter button
} password_button_t;

void password_state_machine_init(void);
int password_state_machine_run(void);
void password_state_machine_button_press(password_button_t button);

#endif // PASSWORD_STATE_MACHINE_H