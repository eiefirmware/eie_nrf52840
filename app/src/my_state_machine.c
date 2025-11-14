/**
 * @file my_state_machine.h
 */

 #include <zephyr/smf.h> 

#include "LED.h"
#include "my_state_machine.h"

/*-------------------------------------------------------
 * Function Prototypes 
 *---------------------------------------------------------*/
static void led_on_state_entry(void*o) {
LED_set(LED0, LED_ON);
}

static enum smf_state_result led_on_state_run(void*o) { 
    if (led_state_object.count > 500) {
        led_state_object.count = 0;
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
    } else {
        led_state_object.count++;
    }
    
    return SMF_EVENT_HANDLED;
}

static void led_off_state_entry(void*0){ 
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result led_off_state_run(void*o) {
        if (led_state_object.count >500) { 
        led_state_object.count = 0; 
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    } else { 
        led_state_object.count++; 
    }

    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum led_state_machine_states {
    LED_ON_STATE, 
    LED_OFF_STATE
};

// zepher uses a struct we define to keep track of the state of the 
// machine... for the LED state machine we are adding a counter to 
//keep track of when we need to change state

typedef struct {
    struct smf_ctx ctx; // context variable used by zephyr to track state machine state
    uint16_t count; // counter to track time in each state
} led_state_object_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
static const struct smf_state led_states[] = {
    [LED_ON_STATE] = SMF_CREATE_STATE(led_on_state_entry, led_on_state_run, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, NULL),
}
void state_machine_init() { 
    led_state_object.count = 0;// Initialization code for the state machine
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
}

int state_machine_run() { 
    // Code to run the state machine
    return smf_run_state(SMF_CTX(&led_state_object));
}