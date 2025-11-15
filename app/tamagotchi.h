/**
 * @file tamagotchi.h
 * @brief Tamagotchi Virtual Pet State Machine
 */

#ifndef TAMAGOTCHI_H
#define TAMAGOTCHI_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TAMA_BUTTON_FEED = 0,    // BTN1
    TAMA_BUTTON_PLAY = 1,    // BTN2
    TAMA_BUTTON_CLEAN = 2,   // BTN3
    TAMA_BUTTON_LIGHT = 3    // BTN4
} tamagotchi_button_t;

void tamagotchi_init(void);
int tamagotchi_run(void);
void tamagotchi_button_press(tamagotchi_button_t button);
void tamagotchi_button_release(tamagotchi_button_t button);

#endif // TAMAGOTCHI_H