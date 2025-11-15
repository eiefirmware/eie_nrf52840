/**
 * @file app_state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "LED.h"
#include "app_state_machine.h"

#define MAX_STRING_LENGTH 256
#define STANDBY_HOLD_TIME_MS 3000
#define SLEEP_MS 1

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum states {
    STATE_CHAR_ENTRY,
    STATE_STRING_BUILD,
    STATE_STRING_READY,
    STATE_STANDBY
};

typedef struct {
    struct smf_ctx ctx;
    uint8_t current_char;
    uint8_t bit_position;
    char string_buffer[MAX_STRING_LENGTH];
    uint8_t string_length;
    enum states previous_state;
    uint32_t btn_hold_counter;
    uint32_t led_counter;
    uint8_t pwm_brightness;
    bool pwm_increasing;
    bool buttons_held[4];
    bool button_events[4];  // Event flags for button presses
} app_state_machine_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
static app_state_machine_t sm;
static const struct smf_state states[];

/*-------------------------------------------------------
 * Helper Functions
 *---------------------------------------------------------*/
static void reset_character(void) {
    printk("[DEBUG] Resetting current character\n");
    sm.current_char = 65; // Start at 'A'
    sm.bit_position = 0;
    printk("[INFO] Character reset to 'A'\n");
}

static void reset_string(void) {
    printk("[DEBUG] Resetting entire string\n");
    sm.string_length = 0;
    for (int i = 0; i < MAX_STRING_LENGTH; i++) {
        sm.string_buffer[i] = '\0';
    }
    printk("[INFO] String buffer cleared\n");
}

// Display current character in binary on LED0-2 (lower 3 bits)
static void display_character_on_leds(void) {
    LED_set(LED0, (sm.current_char & 0x01) ? LED_ON : LED_OFF);
    LED_set(LED1, (sm.current_char & 0x02) ? LED_ON : LED_OFF);
    LED_set(LED2, (sm.current_char & 0x04) ? LED_ON : LED_OFF);
    // LED3 is used for state blinking indicator
}

// Print current character info to console
static void print_current_character(void) {
    printk("[INFO] Current: '%c' (ASCII %d, 0x%02X, 0b", 
           sm.current_char, sm.current_char, sm.current_char);
    for (int i = 7; i >= 0; i--) {
        printk("%d", (sm.current_char >> i) & 1);
    }
    printk(")\n");
}

static void print_string(void) {
    printk("\n╔══════════════════════════════════════════════╗\n");
    printk("║          STRING OUTPUT TO SERIAL             ║\n");
    printk("╚══════════════════════════════════════════════╝\n");
    printk("Length: %d characters\n", sm.string_length);
    printk("String: \"");
    for (int i = 0; i < sm.string_length; i++) {
        printk("%c", sm.string_buffer[i]);
    }
    printk("\"\n");
    printk("Hex: ");
    for (int i = 0; i < sm.string_length; i++) {
        printk("%02X ", sm.string_buffer[i]);
    }
    printk("\n");
    printk("═══════════════════════════════════════════════\n\n");
}

static void blink_led(led_id led) {
    LED_set(led, LED_ON);
    k_msleep(100);
    LED_set(led, LED_OFF);
}

/*-------------------------------------------------------
 * STATE 1: Character Entry (EFFICIENT METHOD)
 *---------------------------------------------------------*/
static void char_entry_entry(void *o) {
    printk("\n╔═══════════════════════════════════════════════╗\n");
    printk("║  STATE 1: CHARACTER ENTRY                    ║\n");
    printk("╚═══════════════════════════════════════════════╝\n");
    printk("Instructions:\n");
    printk("  BTN1: Previous character (←)\n");
    printk("  BTN2: Next character (→)\n");
    printk("  BTN3: Reset to 'A'\n");
    printk("  BTN4: Save character\n");
    printk("  Hold BTN1+BTN2 for 3s: Standby mode\n");
    printk("LED0-2 show character in binary\n");
    printk("LED3 blinking at 1 Hz\n\n");
    
    // Start at 'A' if not already set
    if (sm.current_char < 32 || sm.current_char > 126) {
        sm.current_char = 65; // 'A'
    }
    display_character_on_leds();
    print_current_character();
    sm.led_counter = 0;
}

static enum smf_state_result char_entry_run(void *o) {
    // LED3 blink at 1 Hz (500ms on, 500ms off)
    sm.led_counter++;
    if (sm.led_counter >= 500) {
        static bool led3_state = false;
        led3_state = !led3_state;
        LED_set(LED3, led3_state ? LED_ON : LED_OFF);
        sm.led_counter = 0;
    }
    
    // Check for standby hold (BTN0 + BTN1 for 3 seconds)
    if (sm.buttons_held[APP_BUTTON_0] && sm.buttons_held[APP_BUTTON_1]) {
        sm.btn_hold_counter++;
        if (sm.btn_hold_counter >= STANDBY_HOLD_TIME_MS) {
            printk("[INFO] BTN1+BTN2 held for 3s, entering STANDBY\n");
            sm.previous_state = STATE_CHAR_ENTRY;
            sm.btn_hold_counter = 0;
            smf_set_state(SMF_CTX(&sm), &states[STATE_STANDBY]);
            return SMF_EVENT_HANDLED;
        }
    } else {
        sm.btn_hold_counter = 0;
    }
    
    // Handle button press events
    // BTN0: Previous character (decrement)
    if (sm.button_events[APP_BUTTON_0]) {
        sm.button_events[APP_BUTTON_0] = false;
        printk("[EVENT] BTN1 pressed - Previous character\n");
        
        sm.current_char--;
        // Wrap around: space (32) to tilde (126)
        if (sm.current_char < 32) {
            sm.current_char = 126;
        }
        
        display_character_on_leds();
        print_current_character();
        blink_led(LED0);
    }
    
    // BTN1: Next character (increment)
    if (sm.button_events[APP_BUTTON_1]) {
        sm.button_events[APP_BUTTON_1] = false;
        printk("[EVENT] BTN2 pressed - Next character\n");
        
        sm.current_char++;
        // Wrap around: tilde (126) to space (32)
        if (sm.current_char > 126) {
            sm.current_char = 32;
        }
        
        display_character_on_leds();
        print_current_character();
        blink_led(LED1);
    }
    
    // BTN2: Reset to 'A'
    if (sm.button_events[APP_BUTTON_2]) {
        sm.button_events[APP_BUTTON_2] = false;
        printk("[EVENT] BTN3 pressed - Reset to 'A'\n");
        sm.current_char = 65; // 'A'
        display_character_on_leds();
        print_current_character();
    }
    
    // BTN3: Save character and move to STRING_BUILD
    if (sm.button_events[APP_BUTTON_3]) {
        sm.button_events[APP_BUTTON_3] = false;
        printk("[EVENT] BTN4 pressed - Saving character '%c'\n", sm.current_char);
        
        if (sm.string_length < MAX_STRING_LENGTH - 1) {
            sm.string_buffer[sm.string_length] = sm.current_char;
            sm.string_length++;
            printk("[SUCCESS] Character '%c' saved (string length: %d)\n", 
                   sm.current_char, sm.string_length);
            smf_set_state(SMF_CTX(&sm), &states[STATE_STRING_BUILD]);
        } else {
            printk("[ERROR] String buffer full!\n");
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void char_entry_exit(void *o) {
    printk("[STATE] Exiting CHARACTER ENTRY\n\n");
    LED_set(LED3, LED_OFF);
}

/*-------------------------------------------------------
 * STATE 2: String Building
 *---------------------------------------------------------*/
static void string_build_entry(void *o) {
    printk("\n╔═══════════════════════════════════════════════╗\n");
    printk("║  STATE 2: STRING BUILDING                    ║\n");
    printk("╚═══════════════════════════════════════════════╝\n");
    printk("Current string: \"");
    for (int i = 0; i < sm.string_length; i++) {
        printk("%c", sm.string_buffer[i]);
    }
    printk("\" (%d chars)\n", sm.string_length);
    printk("Instructions:\n");
    printk("  BTN1/BTN2: Select character\n");
    printk("  BTN3: Add character (continue building)\n");
    printk("  BTN4: Finish string\n");
    printk("LED3 blinking at 4 Hz\n\n");
    
    // Reset to 'A' for next character
    sm.current_char = 65;
    display_character_on_leds();
    print_current_character();
    sm.led_counter = 0;
}

static enum smf_state_result string_build_run(void *o) {
    // LED3 blink at 4 Hz (125ms on, 125ms off)
    sm.led_counter++;
    if (sm.led_counter >= 125) {
        static bool led3_state = false;
        led3_state = !led3_state;
        LED_set(LED3, led3_state ? LED_ON : LED_OFF);
        sm.led_counter = 0;
    }
    
    // Check for standby hold
    if (sm.buttons_held[APP_BUTTON_0] && sm.buttons_held[APP_BUTTON_1]) {
        sm.btn_hold_counter++;
        if (sm.btn_hold_counter >= STANDBY_HOLD_TIME_MS) {
            printk("[INFO] BTN1+BTN2 held for 3s, entering STANDBY\n");
            sm.previous_state = STATE_STRING_BUILD;
            sm.btn_hold_counter = 0;
            smf_set_state(SMF_CTX(&sm), &states[STATE_STANDBY]);
            return SMF_EVENT_HANDLED;
        }
    } else {
        sm.btn_hold_counter = 0;
    }
    
    // Handle button press events
    // BTN0: Previous character
    if (sm.button_events[APP_BUTTON_0]) {
        sm.button_events[APP_BUTTON_0] = false;
        printk("[EVENT] BTN1 pressed - Previous character\n");
        
        sm.current_char--;
        if (sm.current_char < 32) {
            sm.current_char = 126;
        }
        
        display_character_on_leds();
        print_current_character();
        blink_led(LED0);
    }
    
    // BTN1: Next character
    if (sm.button_events[APP_BUTTON_1]) {
        sm.button_events[APP_BUTTON_1] = false;
        printk("[EVENT] BTN2 pressed - Next character\n");
        
        sm.current_char++;
        if (sm.current_char > 126) {
            sm.current_char = 32;
        }
        
        display_character_on_leds();
        print_current_character();
        blink_led(LED1);
    }
    
    // BTN2: Add current character and stay in STRING_BUILD
    if (sm.button_events[APP_BUTTON_2]) {
        sm.button_events[APP_BUTTON_2] = false;
        printk("[EVENT] BTN3 pressed - Adding character\n");
        
        if (sm.string_length < MAX_STRING_LENGTH - 1) {
            sm.string_buffer[sm.string_length] = sm.current_char;
            sm.string_length++;
            printk("[SUCCESS] Character '%c' added (total: %d chars)\n", 
                   sm.current_char, sm.string_length);
            
            // Show current string
            printk("[INFO] Current string: \"");
            for (int i = 0; i < sm.string_length; i++) {
                printk("%c", sm.string_buffer[i]);
            }
            printk("\"\n");
            
            // Reset to 'A' for next character
            sm.current_char = 65;
            display_character_on_leds();
            print_current_character();
        } else {
            printk("[ERROR] String buffer full!\n");
        }
    }
    
    // BTN3: Finish string and move to STRING_READY (without adding current char)
    if (sm.button_events[APP_BUTTON_3]) {
        sm.button_events[APP_BUTTON_3] = false;
        printk("[EVENT] BTN4 pressed - Finishing string\n");
        
        if (sm.string_length > 0) {
            printk("[SUCCESS] String complete with %d characters\n", sm.string_length);
            smf_set_state(SMF_CTX(&sm), &states[STATE_STRING_READY]);
        } else {
            printk("[WARN] No characters in string! Add at least one character.\n");
            printk("[INFO] Returning to CHARACTER ENTRY\n");
            reset_character();
            smf_set_state(SMF_CTX(&sm), &states[STATE_CHAR_ENTRY]);
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void string_build_exit(void *o) {
    printk("[STATE] Exiting STRING BUILDING\n\n");
    LED_set(LED3, LED_OFF);
}

/*-------------------------------------------------------
 * STATE 3: String Ready
 *---------------------------------------------------------*/
static void string_ready_entry(void *o) {
    printk("\n╔═══════════════════════════════════════════════╗\n");
    printk("║  STATE 3: STRING READY                       ║\n");
    printk("╚═══════════════════════════════════════════════╝\n");
    printk("String ready to send: \"");
    for (int i = 0; i < sm.string_length; i++) {
        printk("%c", sm.string_buffer[i]);
    }
    printk("\" (%d chars)\n", sm.string_length);
    printk("Instructions:\n");
    printk("  BTN3: Delete string and start over\n");
    printk("  BTN4: Send to serial monitor\n");
    printk("LED3 blinking at 16 Hz\n\n");
    
    sm.led_counter = 0;
}

static enum smf_state_result string_ready_run(void *o) {
    // LED3 blink at 16 Hz (~31ms on, ~31ms off)
    sm.led_counter++;
    if (sm.led_counter >= 31) {
        static bool led3_state = false;
        led3_state = !led3_state;
        LED_set(LED3, led3_state ? LED_ON : LED_OFF);
        sm.led_counter = 0;
    }
    
    // Check for standby hold
    if (sm.buttons_held[APP_BUTTON_0] && sm.buttons_held[APP_BUTTON_1]) {
        sm.btn_hold_counter++;
        if (sm.btn_hold_counter >= STANDBY_HOLD_TIME_MS) {
            printk("[INFO] BTN1+BTN2 held for 3s, entering STANDBY\n");
            sm.previous_state = STATE_STRING_READY;
            sm.btn_hold_counter = 0;
            smf_set_state(SMF_CTX(&sm), &states[STATE_STANDBY]);
            return SMF_EVENT_HANDLED;
        }
    } else {
        sm.btn_hold_counter = 0;
    }
    
    // Handle button press events
    // BTN2: Delete string and go back to CHAR_ENTRY
    if (sm.button_events[APP_BUTTON_2]) {
        sm.button_events[APP_BUTTON_2] = false;
        printk("[EVENT] BTN3 pressed - Deleting string\n");
        reset_string();
        reset_character();
        smf_set_state(SMF_CTX(&sm), &states[STATE_CHAR_ENTRY]);
    }
    
    // BTN3: Print string to serial and go back to CHAR_ENTRY
    if (sm.button_events[APP_BUTTON_3]) {
        sm.button_events[APP_BUTTON_3] = false;
        printk("[EVENT] BTN4 pressed - Sending to serial\n");
        print_string();
        reset_string();
        reset_character();
        smf_set_state(SMF_CTX(&sm), &states[STATE_CHAR_ENTRY]);
    }
    
    return SMF_EVENT_HANDLED;
}

static void string_ready_exit(void *o) {
    printk("[STATE] Exiting STRING READY\n\n");
    LED_set(LED3, LED_OFF);
}

/*-------------------------------------------------------
 * STATE 4: Standby
 *---------------------------------------------------------*/
static void standby_entry(void *o) {
    printk("\n╔═══════════════════════════════════════════════╗\n");
    printk("║  STATE 4: STANDBY MODE                       ║\n");
    printk("╚═══════════════════════════════════════════════╝\n");
    printk("All LEDs pulsing smoothly...\n");
    printk("Press any button to return to previous state.\n\n");
    
    sm.pwm_brightness = 0;
    sm.pwm_increasing = true;
}

static enum smf_state_result standby_run(void *o) {
    // Pulse all LEDs with PWM (smooth breathing effect)
    if (sm.pwm_increasing) {
        sm.pwm_brightness += 2;
        if (sm.pwm_brightness >= 100) {
            sm.pwm_brightness = 100;
            sm.pwm_increasing = false;
        }
    } else {
        if (sm.pwm_brightness >= 2) {
            sm.pwm_brightness -= 2;
        } else {
            sm.pwm_brightness = 0;
            sm.pwm_increasing = true;
        }
    }
    
    LED_pwm(LED0, sm.pwm_brightness);
    LED_pwm(LED1, sm.pwm_brightness);
    LED_pwm(LED2, sm.pwm_brightness);
    LED_pwm(LED3, sm.pwm_brightness);
    
    // Check for any button press to exit
    for (int i = 0; i < 4; i++) {
        if (sm.button_events[i]) {
            sm.button_events[i] = false;
            printk("[EVENT] Button %d pressed - Exiting STANDBY\n", i + 1);
            smf_set_state(SMF_CTX(&sm), &states[sm.previous_state]);
            return SMF_EVENT_HANDLED;
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void standby_exit(void *o) {
    printk("[STATE] Exiting STANDBY MODE\n\n");
    
    // Turn off PWM on all LEDs
    LED_pwm(LED0, 0);
    LED_pwm(LED1, 0);
    LED_pwm(LED2, 0);
    LED_pwm(LED3, 0);
}

/*-------------------------------------------------------
 * State Table
 *---------------------------------------------------------*/
static const struct smf_state states[] = {
    [STATE_CHAR_ENTRY]   = SMF_CREATE_STATE(char_entry_entry, char_entry_run, char_entry_exit, NULL, NULL),
    [STATE_STRING_BUILD] = SMF_CREATE_STATE(string_build_entry, string_build_run, string_build_exit, NULL, NULL),
    [STATE_STRING_READY] = SMF_CREATE_STATE(string_ready_entry, string_ready_run, string_ready_exit, NULL, NULL),
    [STATE_STANDBY]      = SMF_CREATE_STATE(standby_entry, standby_run, standby_exit, NULL, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void app_state_machine_init(void) {
    // Initialize all variables
    sm.current_char = 65; // Start at 'A'
    reset_string();
    sm.previous_state = STATE_CHAR_ENTRY;
    sm.btn_hold_counter = 0;
    sm.led_counter = 0;
    sm.pwm_brightness = 0;
    sm.pwm_increasing = true;
    
    for (int i = 0; i < 4; i++) {
        sm.buttons_held[i] = false;
        sm.button_events[i] = false;
    }
    
    printk("\n\n");
    printk("╔══════════════════════════════════════════════════╗\n");
    printk("║                                                  ║\n");
    printk("║     ASCII String Entry State Machine            ║\n");
    printk("║     Mini Project - EiE                          ║\n");
    printk("║     EFFICIENT CHARACTER SELECTION MODE          ║\n");
    printk("║                                                  ║\n");
    printk("╚══════════════════════════════════════════════════╝\n");
    printk("\n");
    
    // Start in character entry state
    smf_set_initial(SMF_CTX(&sm), &states[STATE_CHAR_ENTRY]);
}

int app_state_machine_run(void) {
    return smf_run_state(SMF_CTX(&sm));
}

void app_state_machine_button_press(app_button_t button) {
    if (button < 4) {
        sm.buttons_held[button] = true;
        sm.button_events[button] = true;
        printk("[INPUT] Button %d pressed\n", button + 1);
    }
}

void app_state_machine_button_release(app_button_t button) {
    if (button < 4) {
        sm.buttons_held[button] = false;
        printk("[INPUT] Button %d released\n", button + 1);
    }
}