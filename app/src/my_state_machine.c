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
static led_state_object_t led_state_object;

// Forward declaration of led_states array
static const struct smf_state led_states[];

/*-------------------------------------------------------
 * State Functions
 *---------------------------------------------------------*/

// Exit function for LED_ON state - turns LED OFF when leaving this state
static void led_on_state_exit(void *o) {
    LED_set(LED0, LED_OFF);
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

// Exit function for LED_OFF state - turns LED ON when leaving this state
static void led_off_state_exit(void *o) { 
    LED_set(LED0, LED_ON);
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
// SMF_CREATE_STATE(entry, run, exit, parent, initial)
// Now using NULL for entry and providing exit functions
static const struct smf_state led_states[] = {
    [LED_ON_STATE] = SMF_CREATE_STATE(NULL, led_on_state_run, led_on_state_exit, NULL, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(NULL, led_off_state_run, led_off_state_exit, NULL, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void state_machine_init(void) { 
    led_state_object.count = 0;
    // Start in LED_OFF state, so the exit function will turn LED ON
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
}

int state_machine_run(void) { 
    return smf_run_state(SMF_CTX(&led_state_object));
}