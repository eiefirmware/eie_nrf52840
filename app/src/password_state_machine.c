/**
 * @file password_state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "password_state_machine.h"

/*-------------------------------------------------------
 * Configuration
 *---------------------------------------------------------*/
#define PASSWORD_LENGTH 4
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const uint8_t correct_password[PASSWORD_LENGTH] = {0, 0, 0, 0}; // BTN1 four times

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum password_states {
    STATE_LOCKED,
    STATE_UNLOCKED
};

typedef struct {
    struct smf_ctx ctx;
    uint8_t password_buffer[PASSWORD_LENGTH];
    uint8_t password_index;
    bool button_event[4];  // Track button press events
} password_state_machine_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
static password_state_machine_t psm_object;
static const struct smf_state password_states[];

/*-------------------------------------------------------
 * Helper Functions
 *---------------------------------------------------------*/
static void reset_password_entry(void) {
    psm_object.password_index = 0;
    for(int i = 0; i < PASSWORD_LENGTH; i++) {
        psm_object.password_buffer[i] = 0xFF;
    }
}

static bool check_password(void) {
    if(psm_object.password_index != PASSWORD_LENGTH) {
        return false;
    }
    
    for(int i = 0; i < PASSWORD_LENGTH; i++) {
        if(psm_object.password_buffer[i] != correct_password[i]) {
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------
 * State: LOCKED
 *---------------------------------------------------------*/
static void locked_state_entry(void *o) {
    printk("\n=== Entering LOCKED State ===\n");
    printk("LED ON - System Locked\n");
    printk("Enter password: BTN1, BTN1, BTN1, BTN1\n");
    printk("Press BTN4 to submit\n\n");
    
    gpio_pin_set_dt(&led0, 1);  // LED ON
    reset_password_entry();
}

static enum smf_state_result locked_state_run(void *o) {
    // Handle BTN0 (digit 0)
    if(psm_object.button_event[PASSWORD_BUTTON_0]) {
        psm_object.button_event[PASSWORD_BUTTON_0] = false;
        if(psm_object.password_index < PASSWORD_LENGTH) {
            psm_object.password_buffer[psm_object.password_index] = 0;
            psm_object.password_index++;
            printk("Entered: BTN1 (digit %d/%d)\n", 
                   psm_object.password_index, PASSWORD_LENGTH);
        }
    }
    
    // Handle BTN1 (digit 1)
    if(psm_object.button_event[PASSWORD_BUTTON_1]) {
        psm_object.button_event[PASSWORD_BUTTON_1] = false;
        if(psm_object.password_index < PASSWORD_LENGTH) {
            psm_object.password_buffer[psm_object.password_index] = 1;
            psm_object.password_index++;
            printk("Entered: BTN2 (digit %d/%d)\n", 
                   psm_object.password_index, PASSWORD_LENGTH);
        }
    }
    
    // Handle BTN2 (digit 2)
    if(psm_object.button_event[PASSWORD_BUTTON_2]) {
        psm_object.button_event[PASSWORD_BUTTON_2] = false;
        if(psm_object.password_index < PASSWORD_LENGTH) {
            psm_object.password_buffer[psm_object.password_index] = 2;
            psm_object.password_index++;
            printk("Entered: BTN3 (digit %d/%d)\n", 
                   psm_object.password_index, PASSWORD_LENGTH);
        }
    }
    
    // Handle BTN3 (Enter button)
    if(psm_object.button_event[PASSWORD_BUTTON_3]) {
        psm_object.button_event[PASSWORD_BUTTON_3] = false;
        
        if(check_password()) {
            printk("\n*** PASSWORD CORRECT! ***\n");
            smf_set_state(SMF_CTX(&psm_object), &password_states[STATE_UNLOCKED]);
        } else {
            printk("\n*** PASSWORD INCORRECT! ***\n");
            printk("Try again...\n\n");
            reset_password_entry();
        }
    }
    
    return SMF_EVENT_HANDLED;
}

static void locked_state_exit(void *o) {
    printk("=== Exiting LOCKED State ===\n\n");
}

/*-------------------------------------------------------
 * State: UNLOCKED
 *---------------------------------------------------------*/
static void unlocked_state_entry(void *o) {
    printk("=== Entering UNLOCKED State ===\n");
    printk("LED OFF - System Unlocked\n");
    printk("Press any button to lock again\n\n");
    
    gpio_pin_set_dt(&led0, 0);  // LED OFF
}

static enum smf_state_result unlocked_state_run(void *o) {
    // Any button press transitions back to LOCKED
    if(psm_object.button_event[PASSWORD_BUTTON_0] ||
       psm_object.button_event[PASSWORD_BUTTON_1] ||
       psm_object.button_event[PASSWORD_BUTTON_2] ||
       psm_object.button_event[PASSWORD_BUTTON_3]) {
        
        // Clear all button events
        for(int i = 0; i < 4; i++) {
            psm_object.button_event[i] = false;
        }
        
        printk("Button pressed - Locking system...\n");
        smf_set_state(SMF_CTX(&psm_object), &password_states[STATE_LOCKED]);
    }
    
    return SMF_EVENT_HANDLED;
}

static void unlocked_state_exit(void *o) {
    printk("=== Exiting UNLOCKED State ===\n\n");
}

/*-------------------------------------------------------
 * State Table
 *---------------------------------------------------------*/
static const struct smf_state password_states[] = {
    [STATE_LOCKED]   = SMF_CREATE_STATE(locked_state_entry, locked_state_run, 
                                        locked_state_exit, NULL, NULL),
    [STATE_UNLOCKED] = SMF_CREATE_STATE(unlocked_state_entry, unlocked_state_run, 
                                        unlocked_state_exit, NULL, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void password_state_machine_init(void) {
    // Initialize GPIO
    if(!gpio_is_ready_dt(&led0)) {
        printk("Error: LED not ready\n");
        return;
    }
    
    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if(ret < 0) {
        printk("Error: Failed to configure LED\n");
        return;
    }
    
    // Initialize state machine variables
    reset_password_entry();
    for(int i = 0; i < 4; i++) {
        psm_object.button_event[i] = false;
    }
    
    printk("\n*** Password Lock System Initialized ***\n");
    printk("Correct password: BTN1, BTN1, BTN1, BTN1\n\n");
    
    // Start in LOCKED state
    smf_set_initial(SMF_CTX(&psm_object), &password_states[STATE_LOCKED]);
}

int password_state_machine_run(void) {
    return smf_run_state(SMF_CTX(&psm_object));
}

void password_state_machine_button_press(password_button_t button) {
    if(button < 4) {
        psm_object.button_event[button] = true;
    }
}