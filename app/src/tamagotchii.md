/**
 * @file tamagotchi.c
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "LED.h"
#include "tamagotchi.h"

#define MAX_STAT 10

typedef struct {
    struct smf_ctx ctx;
    uint8_t hunger;
    uint8_t happiness;
    uint32_t timer;
    bool button_events[4];
} tamagotchi_t;

static tamagotchi_t tama;

void tamagotchi_init(void) {
    tama.hunger = 8;
    tama.happiness = 8;
    tama.timer = 0;
    
    for (int i = 0; i < 4; i++) {
        tama.button_events[i] = false;
    }
    
    printk("\n🐣 TAMAGOTCHI HATCHED!\n");
    printk("BTN1: Feed 🍚\n");
    printk("BTN2: Play 🎮\n");
    printk("BTN3: Clean 🧹\n");
    printk("BTN4: Status 📊\n\n");
}

int tamagotchi_run(void) {
    // Simple timer
    tama.timer++;
    
    // Decrease stats slowly
    if (tama.timer % 30000 == 0) {
        if (tama.hunger > 0) tama.hunger--;
        if (tama.happiness > 0) tama.happiness--;
    }
    
    // Handle buttons
    if (tama.button_events[TAMA_BUTTON_FEED]) {
        tama.button_events[TAMA_BUTTON_FEED] = false;
        if (tama.hunger < MAX_STAT) {
            tama.hunger += 2;
            if (tama.hunger > MAX_STAT) tama.hunger = MAX_STAT;
            printk("🍚 Fed! Hunger: %d/10\n", tama.hunger);
        }
    }
    
    if (tama.button_events[TAMA_BUTTON_PLAY]) {
        tama.button_events[TAMA_BUTTON_PLAY] = false;
        if (tama.happiness < MAX_STAT) {
            tama.happiness += 2;
            if (tama.happiness > MAX_STAT) tama.happiness = MAX_STAT;
            printk("🎮 Played! Happiness: %d/10\n", tama.happiness);
        }
    }
    
    if (tama.button_events[TAMA_BUTTON_CLEAN]) {
        tama.button_events[TAMA_BUTTON_CLEAN] = false;
        printk("🧹 Cleaned!\n");
    }
    
    if (tama.button_events[TAMA_BUTTON_LIGHT]) {
        tama.button_events[TAMA_BUTTON_LIGHT] = false;
        printk("📊 STATUS: Hunger:%d/10 Happy:%d/10\n", tama.hunger, tama.happiness);
    }
    
    // LED indicators
    LED_set(LED0, tama.hunger < 5 ? LED_ON : LED_OFF);
    LED_set(LED1, tama.happiness < 5 ? LED_ON : LED_OFF);
    
    return 0;
}

void tamagotchi_button_press(tamagotchi_button_t button) {
    if (button < 4) {
        tama.button_events[button] = true;
    }
}

void tamagotchi_button_release(tamagotchi_button_t button) {
    // Nothing needed for now
}