/**
 * @file tamagotchi.c
 * @brief Tamagotchi Virtual Pet Implementation
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "LED.h"
#include "tamagotchi.h"

#define MAX_STAT 10
#define POOP_THRESHOLD 3
#define SLEEP_HOUR 22  // 10 PM
#define WAKE_HOUR 8    // 8 AM

/*-------------------------------------------------------
 * ASCII Art Faces
 *---------------------------------------------------------*/
const char *TAMA_NORMAL = 
"⠀⠀⠀⠀⢀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣶⣾⣿⣿⣿⣶⣄⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⣠⣾⣿⣿⣿⣿⣿⣷⣦⡀⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⡀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⡿⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠈⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠸⣿⣿⣿⣿⣿⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⣦⣤⣴⣾⣿⣿⣿⣿⣿⣿⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⣿⠿⠿⠿⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢸⠃⠀⠀⠀⣠⣄⡀⠀⠉⠉⠉⠉⠉⠉⠉⠉⠁⣤⣄⠀⠀⠘⣇⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⡿⠀⠀⠀⠀⠿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠿⠿⠀⠀⠀⢹⣤⣤⣄⡀⠀\n"
"⢠⡴⠒⢺⡇⠀⠐⡌⢆⠀⠀⠀⢠⡀⠀⠀⣤⡀⠀⠀⣀⠀⠀⠐⣌⠒⠀⠈⣇⠀⠀⠙⢷\n"
"⣿⠀⠀⢸⡇⠀⠀⠈⠀⠀⠀⠀⠈⠓⠶⠚⠉⠓⠶⠴⠟⠀⠀⠀⠀⠁⠀⠀⣿⢦⣤⣤⡾\n"
"⠈⠓⠶⢾⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠈⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⠇⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢠⠞⠙⢦⣄⣀⡀⠀⠀⠀⢀⣀⣀⣀⣀⣀⣀⣀⣀⣤⣤⡶⠛⠁⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⣿⠀⠀⠀⢀⡽⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⡿⠁⠀⠀⠈⣷⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠈⠛⠒⠚⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣤⣀⣠⡴⠟⠀⠀⠀⠀⠀⠀⠀\n";

const char *TAMA_EATING = 
"⠀⠀⠀⠀⣀⣤⣴⣶⣶⣶⣦⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⣠⣾⣿⣿⣿⣿⣿⣿⢿⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⢀⣾⣿⣿⣿⣿⣿⣿⣿⣅⢀⣽⣿⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⠛⠁⠀⠀⣴⣶⡄⠀⣶⣶⡄⠀⣴⣶⡄\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⣀⠀⠙⠋⠁⠀⠉⠋⠁⠀⠙⠋⠀\n"
"⠸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠈⠙⠿⣿⣿⣿⣿⣿⣿⣿⠿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠉⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";

const char *TAMA_HAPPY = 
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠑⠂⠒⠋\n"
"⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⣤⣤⣤⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣤⣶⣶⣶⣤⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⢀⣶⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀⠀⠀⠀⠀⣰⣿⣿⣿⣿⣿⣿⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⢘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡃⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣴⣴⣶⣶⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡃⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⣨⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⢠⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀⠀⠀⠀\n"
"⠀⣸⡿⠁⠀⠀⠀⠀⣀⣉⣉⠉⠙⠋⠛⠛⠛⠛⠛⠛⠛⠛⠛⠙⠉⠉⠉⠉⠉⠉⠉⠉⠉⠛⣷⡀⠀⠀⠀\n"
"⢠⡿⠀⠀⢠⣶⣿⣿⣿⣿⣿⡛⠻⣦⣄⠀⠀⠀⠀⠀⠀⢀⣤⣶⣾⣿⣶⣶⣦⣄⡀⠀⠀⠀⠀⠸⣷⡀⠀⠀\n"
"⣾⠇⠀⠀⣿⣿⠀⣿⣿⣿⣿⡇⠀⠈⢿⡄⠀⠀⠀⠀⣰⣟⠈⣿⣿⣿⣿⣇⠀⠉⢿⡆⠀⠀⠀⠀⢸⣧⠀⠀\n"
"⣿⠀⠀⠀⣿⣿⣶⣿⣿⣿⣿⡇⠀⠀⣼⠇⠀⠀⠀⠀⣿⣿⣼⣿⣿⣿⣿⣿⠀⠀⢨⣿⠀⠀⠀⠀⠀⣿⠀⠀\n"
"⣿⡄⠀⠀⠘⠿⣿⣿⣿⣿⡿⣀⣠⣼⠟⠀⠀⠀⠀⠀⠙⢿⣿⣿⣿⣿⣿⡏⣀⣠⡾⠋⠀⠀⠀⠀⠀⣿⠀⠀\n"
"⢸⣧⠀⠀⡠⠐⠚⡩⢙⠛⠛⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠟⠟⠛⠫⢉⠐⠢⢄⠀⠀⠀⣼⡏⠀⠀\n"
"⠀⢻⣦⡘⡠⢁⠃⠤⢡⠃⠀⠀⠀⠀⢰⣆⣀⠀⢀⣀⣶⠀⠀⠀⠀⠀⠀⠸⡠⢁⠊⠔⡢⠀⢀⣼⠏⠀⠀⠀\n"
"⠀⠀⠉⠻⣮⣅⡈⠈⠁⠀⠀⠀⠀⠀⠀⠉⠛⠛⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀⠈⠀⠁⢈⣠⣶⠟⠉⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠉⠛⠛⠿⠶⣦⣤⣤⣤⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣤⣤⣤⣶⣶⠶⠿⠛⠋⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⡟⠀⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠀⢸⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⡟⠀⣰⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣶⡀⠹⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠷⠟⢻⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡻⠷⠿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⡇⠀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡃⣼⡟⠛⠛⠛⠛⠛⠛⢻⡆⢠⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⠋⠀⠀⠀⠀⠀⠀⠀⠘⠻⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";

const char *TAMA_HUNGRY = "(つ╥﹏╥)つ  FEED ME!";

const char *TAMA_SAD = "(｡ᵕ ◞ _◟)  Need some love...";

const char *TAMA_SICK = "\"( – ⌓ – )  💊 NEED MEDICINE!";

const char *TAMA_SLEEPING = "⋆｡˚ ☁︎ᶻ 𝗓 𐰁 ˚｡⋆｡˚☽˚｡⋆ᶻ 𝗓 𐰁";

const char *TAMA_POOP = "💩";

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum states {
    STATE_IDLE,
    STATE_HUNGRY,
    STATE_SAD,
    STATE_SLEEPING,
    STATE_DIRTY,
    STATE_SICK,
    STATE_ATTENTION,
    STATE_DEAD
};

typedef struct {
    struct smf_ctx ctx;
    
    // Core stats (0-10)
    uint8_t hunger;      // 10 = full, 0 = starving
    uint8_t happiness;   // 10 = happy, 0 = very sad
    uint8_t health;      // 10 = healthy, 0 = dead
    uint8_t discipline;  // 0-10
    
    // Conditions
    uint8_t poop_count;  // 0-3
    uint8_t weight;      // in grams
    uint32_t age;        // in seconds
    bool is_sleeping;
    bool lights_on;
    bool is_sick;
    
    // Timers
    uint32_t hunger_timer;
    uint32_t happiness_timer;
    uint32_t poop_timer;
    uint32_t sick_check_timer;
    uint32_t led_blink_timer;
    
    // Previous state for attention
    enum states previous_state;
    
    // Button states
    bool buttons_held[4];
    bool button_events[4];
    
} tamagotchi_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
static tamagotchi_t tama;
static const struct smf_state states[];

/*-------------------------------------------------------
 * Helper Functions
 *---------------------------------------------------------*/
static void print_stats(void) {
    printk("\n═══════════════════════════════════════\n");
    printk("📊 TAMAGOTCHI STATUS\n");
    printk("═══════════════════════════════════════\n");
    printk("🍚 Hunger:    ");
    for (int i = 0; i < tama.hunger; i++) printk("♥");
    printk(" (%d/10)\n", tama.hunger);
    
    printk("😊 Happiness: ");
    for (int i = 0; i < tama.happiness; i++) printk("♥");
    printk(" (%d/10)\n", tama.happiness);
    
    printk("❤️  Health:    ");
    for (int i = 0; i < tama.health; i++) printk("♥");
    printk(" (%d/10)\n", tama.health);
    
    printk("⚖️  Weight:    %d g\n", tama.weight);
    printk("🎂 Age:       %d seconds\n", tama.age);
    printk("💩 Poop:      %d\n", tama.poop_count);
    
    if (tama.is_sick) printk("💀 STATUS:    SICK!\n");
    if (tama.is_sleeping) printk("😴 STATUS:    Sleeping\n");
    printk("═══════════════════════════════════════\n\n");
}

static void print_face(const char *face) {
    printk("\n%s\n", face);
}

static void update_leds(void) {
    // LED0: Hunger indicator (blinks faster when hungry)
    if (tama.hunger <= 3) {
        LED_set(LED0, (tama.led_blink_timer / 250) % 2 ? LED_ON : LED_OFF);
    } else if (tama.hunger <= 6) {
        LED_set(LED0, (tama.led_blink_timer / 500) % 2 ? LED_ON : LED_OFF);
    } else {
        LED_set(LED0, LED_OFF);
    }
    
    // LED1: Happiness indicator (blinks faster when sad)
    if (tama.happiness <= 3) {
        LED_set(LED1, (tama.led_blink_timer / 250) % 2 ? LED_ON : LED_OFF);
    } else if (tama.happiness <= 6) {
        LED_set(LED1, (tama.led_blink_timer / 500) % 2 ? LED_ON : LED_OFF);
    } else {
        LED_set(LED1, LED_OFF);
    }
    
    // LED2: Alert (poop or sick)
    if (tama.poop_count > 0 || tama.is_sick) {
        LED_set(LED2, (tama.led_blink_timer / 200) % 2 ? LED_ON : LED_OFF);
    } else {
        LED_set(LED2, LED_OFF);
    }
    
    // LED3: Sleep status
    LED_set(LED3, tama.is_sleeping ? LED_ON : LED_OFF);
}

static void feed_meal(void) {
    if (tama.hunger < MAX_STAT) {
        tama.hunger += 3;
        if (tama.hunger > MAX_STAT) tama.hunger = MAX_STAT;
        tama.weight += 1;
        printk("🍚 Fed meal! Hunger: %d/10, Weight: %dg\n", tama.hunger, tama.weight);
        print_face(TAMA_HAPPY);
    } else {
        printk("❌ Not hungry right now!\n");
    }
}

static void feed_snack(void) {
    if (tama.happiness < MAX_STAT) {
        tama.happiness += 2;
        if (tama.happiness > MAX_STAT) tama.happiness = MAX_STAT;
        tama.weight += 2;  // Snacks add more weight
        printk("🍰 Fed snack! Happiness: %d/10, Weight: %dg\n", tama.happiness, tama.weight);
        print_face(TAMA_HAPPY);
    } else {
        printk("❌ Already happy!\n");
    }
}

static void play_game(void) {
    if (!tama.is_sleeping) {
        if (tama.happiness < MAX_STAT) {
            tama.happiness += 3;
            if (tama.happiness > MAX_STAT) tama.happiness = MAX_STAT;
            tama.weight -= 1;  // Playing reduces weight
            if (tama.weight < 20) tama.weight = 20;
            printk("🎮 Played game! Happiness: %d/10\n", tama.happiness);
            print_face(TAMA_HAPPY);
        } else {
            printk("❌ Already very happy!\n");
        }
    } else {
        printk("❌ Can't play while sleeping!\n");
    }
}

static void clean_poop(void) {
    if (tama.poop_count > 0) {
        tama.poop_count = 0;
        printk("🧹 Cleaned up! Nice and fresh!\n");
        print_face(TAMA_NORMAL);
    } else {
        printk("❌ Nothing to clean!\n");
    }
}

static void give_medicine(void) {
    if (tama.is_sick) {
        tama.is_sick = false;
        tama.health += 3;
        if (tama.health > MAX_STAT) tama.health = MAX_STAT;
        printk("💊 Gave medicine! Health: %d/10\n", tama.health);
        print_face(TAMA_NORMAL);
    } else {
        printk("❌ Not sick!\n");
    }
}

static void toggle_lights(void) {
    tama.lights_on = !tama.lights_on;
    
    if (tama.is_sleeping && tama.lights_on) {
        printk("💡 Lights ON - Waking up!\n");
        tama.is_sleeping = false;
        print_face(TAMA_NORMAL);
    } else if (!tama.is_sleeping && !tama.lights_on) {
        printk("💡 Lights OFF - Going to sleep!\n");
        tama.is_sleeping = true;
        print_face(TAMA_SLEEPING);
    } else if (tama.lights_on) {
        printk("💡 Lights already on!\n");
    } else {
        printk("💡 Lights already off!\n");
    }
}

/*-------------------------------------------------------
 * STATE: IDLE
 *---------------------------------------------------------*/
static void idle_entry(void *o) {
    printk("\n🐣 TAMAGOTCHI - IDLE STATE\n");
    print_face(TAMA_NORMAL);
}

static enum smf_state_result idle_run(void *o) {
    // Update timers
    tama.hunger_timer++;
    tama.happiness_timer++;
    tama.poop_timer++;
    tama.sick_check_timer++;
    tama.led_blink_timer++;
    tama.age++;
    
    // Decrease hunger every 30 seconds
    if (tama.hunger_timer >= 30000 && !tama.is_sleeping) {
        tama.hunger--;
        if (tama.hunger == 0) tama.hunger = 0;
        tama.hunger_timer = 0;
        printk("⏰ Getting hungry... (%d/10)\n", tama.hunger);
    }
    
    // Decrease happiness every 45 seconds
    if (tama.happiness_timer >= 45000 && !tama.is_sleeping) {
        tama.happiness--;
        if (tama.happiness == 0) tama.happiness = 0;
        tama.happiness_timer = 0;
        printk("⏰ Getting bored... (%d/10)\n", tama.happiness);
    }
    
    // Poop every 60 seconds
    if (tama.poop_timer >= 60000 && !tama.is_sleeping) {
        if (tama.poop_count < POOP_THRESHOLD) {
            tama.poop_count++;
            printk("💩 *POOP* Clean me up! (%d poops)\n", tama.poop_count);
            print_face(TAMA_POOP);
            tama.poop_timer = 0;
        }
    }
    
    // Check for sickness every 2 minutes
    if (tama.sick_check_timer >= 120000) {
        // Get sick if too much poop or low health
        if ((tama.poop_count >= POOP_THRESHOLD || tama.health <= 3) && !tama.is_sick) {
            tama.is_sick = true;
            tama.health -= 2;
            printk("💀 OH NO! Tamagotchi got SICK!\n");
            print_face(TAMA_SICK);
        }
        tama.sick_check_timer = 0;
    }
    
    // Update LED indicators
    update_leds();
    
    // Check for state transitions
    if (tama.health <= 0) {
        smf_set_state(SMF_CTX(&tama), &states[STATE_DEAD]);
        return SMF_EVENT_HANDLED;
    }
    
    if (tama.is_sick) {
        smf_set_state(SMF_CTX(&tama), &states[STATE_SICK]);
        return SMF_EVENT_HANDLED;
    }
    
    if (tama.poop_count >= POOP_THRESHOLD) {
        smf_set_state(SMF_CTX(&tama), &states[STATE_DIRTY]);
        return SMF_EVENT_HANDLED;
    }
    
    if (tama.hunger <= 2) {
        smf_set_state(SMF_CTX(&tama), &states[STATE_HUNGRY]);
        return SMF_EVENT_HANDLED;
    }
    
    if (tama.happiness <= 2) {
        smf_set_state(SMF_CTX(&tama), &states[STATE_SAD]);
        return SMF_EVENT_HANDLED;
    }
    
    // Handle button presses
    if (tama.button_events[TAMA_BUTTON_FEED]) {
        tama.button_events[TAMA_BUTTON_FEED] = false;
        feed_meal();
    }
    
    if (tama.button_events[TAMA_BUTTON_PLAY]) {
        tama.button_events[TAMA_BUTTON_PLAY] = false;
        if (tama.is_sleeping) {
            feed_snack();
        } else {
            play_game();
        }
    }
    
    if (tama.button_events[TAMA_BUTTON_CLEAN]) {
        tama.button_events[TAMA_BUTTON_CLEAN] = false;
        if (tama.poop_count > 0) {
            clean_poop();
        } else {
            give_medicine();
        }
    }
    
    if (tama.button_events[TAMA_BUTTON_LIGHT]) {
        tama.button_events[TAMA_BUTTON_LIGHT] = false;
        toggle_lights();
    }
    
    return SMF_EVENT_HANDLED;
}

static void idle_exit(void *o) {
    printk("Leaving IDLE state\n");
}

/*-------------------------------------------------------
 * STATE: HUNGRY
 *---------------------------------------------------------*/
static void hungry_entry(void *o) {
    printk("\n🍚 TAMAGOTCHI IS HUNGRY!\n");
    print_face(TAMA_HUNGRY);
}

static enum smf_state_result hungry_run(void *o) {
    // Continue idle behavior
    return idle_run(o);
}

static void hungry_exit(void *o) {
    printk("No longer hungry!\n");
}

/*-------------------------------------------------------
 * STATE: SAD
 *---------------------------------------------------------*/
static void sad_entry(void *o) {
    printk("\n😢 TAMAGOTCHI IS SAD!\n");
    print_face(TAMA_SAD);
}

static enum smf_state_result sad_run(void *o) {
    return idle_run(o);
}

static void sad_exit(void *o) {
    printk("Feeling better!\n");
}

/*-------------------------------------------------------
 * STATE: DIRTY
 *---------------------------------------------------------*/
static void dirty_entry(void *o) {
    printk("\n💩 TAMAGOTCHI NEEDS CLEANING!\n");
    printk("Poop count: %d - Clean me up!\n", tama.poop_count);
}

static enum smf_state_result dirty_run(void *o) {
    // Flash LED2 rapidly
    LED_set(LED2, (tama.led_blink_timer / 100) % 2 ? LED_ON : LED_OFF);
    tama.led_blink_timer++;
    
    if (tama.button_events[TAMA_BUTTON_CLEAN]) {
        tama.button_events[TAMA_BUTTON_CLEAN] = false;
        clean_poop();
        smf_set_state(SMF_CTX(&tama), &states[STATE_IDLE]);
        return SMF_EVENT_HANDLED;
    }
    
    return idle_run(o);
}

static void dirty_exit(void *o) {
    printk("All clean!\n");
}

/*-------------------------------------------------------
 * STATE: SICK
 *---------------------------------------------------------*/
static void sick_entry(void *o) {
    printk("\n💀 TAMAGOTCHI IS SICK!\n");
    printk("Give medicine NOW!\n");
    print_face(TAMA_SICK);
}

static enum smf_state_result sick_run(void *o) {
    // Flash all LEDs
    LED_set(LED0, (tama.led_blink_timer / 100) % 2 ? LED_ON : LED_OFF);
    LED_set(LED1, (tama.led_blink_timer / 100) % 2 ? LED_ON : LED_OFF);
    LED_set(LED2, LED_ON);
    LED_set(LED3, (tama.led_blink_timer / 100) % 2 ? LED_ON : LED_OFF);
    tama.led_blink_timer++;
    
    // Health deteriorates when sick
    static uint32_t sick_timer = 0;
    sick_timer++;
    if (sick_timer >= 10000) {
        tama.health--;
        printk("⚠️  Health dropping! %d/10\n", tama.health);
        sick_timer = 0;
        
        if (tama.health <= 0) {
            smf_set_state(SMF_CTX(&tama), &states[STATE_DEAD]);
            return SMF_EVENT_HANDLED;
        }
    }
    
    if (tama.button_events[TAMA_BUTTON_CLEAN]) {
        tama.button_events[TAMA_BUTTON_CLEAN] = false;
        give_medicine();
        if (!tama.is_sick) {
            smf_set_state(SMF_CTX(&tama), &states[STATE_IDLE]);
            return SMF_EVENT_HANDLED;
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void sick_exit(void *o) {
    printk("Feeling healthy again!\n");
}

/*-------------------------------------------------------
 * STATE: DEAD
 *---------------------------------------------------------*/
static void dead_entry(void *o) {
    printk("\n💀💀💀 TAMAGOTCHI DIED! 💀💀💀\n");
    printk("Press any button to restart...\n");
    
    // All LEDs off
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result dead_run(void *o) {
    // Check for any button to restart
    for (int i = 0; i < 4; i++) {
        if (tama.button_events[i]) {
            tama.button_events[i] = false;
            tamagotchi_init();
            return SMF_EVENT_HANDLED;
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void dead_exit(void *o) {
    printk("Restarting...\n");
}

/*-------------------------------------------------------
 * State Table
 *---------------------------------------------------------*/
static const struct smf_state states[] = {
    [STATE_IDLE]     = SMF_CREATE_STATE(idle_entry, idle_run, idle_exit, NULL, NULL),
    [STATE_HUNGRY]   = SMF_CREATE_STATE(hungry_entry, hungry_run, hungry_exit, NULL, NULL),
    [STATE_SAD]      = SMF_CREATE_STATE(sad_entry, sad_run, sad_exit, NULL, NULL),
    [STATE_SLEEPING] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [STATE_DIRTY]    = SMF_CREATE_STATE(dirty_entry, dirty_run, dirty_exit, NULL, NULL),
    [STATE_SICK]     = SMF_CREATE_STATE(sick_entry, sick_run, sick_exit, NULL, NULL),
    [STATE_ATTENTION] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [STATE_DEAD]     = SMF_CREATE_STATE(dead_entry, dead_run, dead_exit, NULL, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void tamagotchi_init(void) {
    // Initialize stats
    tama.hunger = 8;
    tama.happiness = 8;
    tama.health = 10;
    tama.discipline = 5;
    tama.poop_count = 0;
    tama.weight = 25;
    tama.age = 0;
    tama.is_sleeping = false;
    tama.lights_on = true;
    tama.is_sick = false;
    
    // Reset timers
    tama.hunger_timer = 0;
    tama.happiness_timer = 0;
    tama.poop_timer = 0;
    tama.sick_check_timer = 0;
    tama.led_blink_timer = 0;
    
    // Reset buttons
    for (int i = 0; i < 4; i++) {
        tama.buttons_held[i] = false;
        tama.button_events[i] = false;
    }
    
    printk("\n\n");
    printk("╔══════════════════════════════════════════════════╗\n");
    printk("║                                                  ║\n");
    printk("║           🐣 TAMAGOTCHI VIRTUAL PET 🐣          ║\n");
    printk("║                  Mini Project                    ║\n");
    printk("║                                                  ║\n");
    printk("╚══════════════════════════════════════════════════╝\n");
    printk("\n");
    printk("🔘 BTN1: Feed Meal 🍚\n");
    printk("🔘 BTN2: Play / Snack 🎮\n");
    printk("🔘 BTN3: Clean / Medicine 🧹\n");
    printk("🔘 BTN4: Lights / Status 💡\n");
    printk("\n");
    
    print_face(TAMA_NORMAL);
    print_stats();
    
    smf_set_initial(SMF_CTX(&tama), &states[STATE_IDLE]);
}

int tamagotchi_run(void) {
    return smf_run_state(SMF_CTX(&tama));
}

void tamagotchi_button_press(tamagotchi_button_t button) {
    if (button < 4) {
        tama.buttons_held[button] = true;
        tama.button_events[button] = true;
        
        // Also print status on button 4
        if (button == TAMA_BUTTON_LIGHT) {
            print_stats();
        }
    }
}

void tamagotchi_button_release(tamagotchi_button_t button) {
    if (button < 4) {
        tama.buttons_held[button] = false;
    }
}