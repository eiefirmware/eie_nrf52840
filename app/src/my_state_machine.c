/**
 * @file my_state_machine.c
 */

#include <zephyr/smf.h>
#include "LED.h"
#include "my_state_machine.h"

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum led_state_machine_states {
    LED_ON_STATE, 
    LED_OFF_STATE
};

// Struct to keep track of state machine state
typedef struct {
    struct smf_ctx ctx; // Context variable used by Zephyr SMF
    uint16_t count;     // Counter to track time in each state
} led_state_object_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
// FIXED: Declare the actual state object variable
static led_state_object_t led_state_object;

/*-------------------------------------------------------
 * State Functions
 *---------------------------------------------------------*/
static void led_on_state_entry(void *o) {
    LED_set(LED0, LED_ON);
}

static enum smf_state_result led_on_state_run(void *o) { 
    if (led_state_object.count > 500) {
        led_state_object.count = 0;
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
    } else {
        led_state_object.count++;
    }
    
    return SMF_EVENT_HANDLED;
}

// FIXED: Changed void*0 to void*o
static void led_off_state_entry(void *o) { 
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result led_off_state_run(void *o) {
    if (led_state_object.count > 500) { 
        led_state_object.count = 0; 
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    } else { 
        led_state_object.count++; 
    }

    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State Table
 *---------------------------------------------------------*/
// FIXED: Added semicolon at the end
static const struct smf_state led_states[] = {
    [LED_ON_STATE] = SMF_CREATE_STATE(led_on_state_entry, led_on_state_run, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void state_machine_init(void) { 
    led_state_object.count = 0;
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
}

int state_machine_run(void) { 
    return smf_run_state(SMF_CTX(&led_state_object));
}