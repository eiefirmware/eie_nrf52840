/**
 * @file app_state_machine.h
 */

#ifndef APP_STATE_MACHINE_H
#define APP_STATE_MACHINE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_BUTTON_0 = 0,
    APP_BUTTON_1 = 1,
    APP_BUTTON_2 = 2,
    APP_BUTTON_3 = 3
} app_button_t;

void app_state_machine_init(void);
int app_state_machine_run(void);
void app_state_machine_button_press(app_button_t button);
void app_state_machine_button_release(app_button_t button);

#endif // APP_STATE_MACHINE_H