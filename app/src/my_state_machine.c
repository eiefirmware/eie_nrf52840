/**
 * @file my_state_machine.c
 */

#include <zephyr/smf.h>
#include "LED.h"
#include "my_state_machine.h"

/*-------------------------------------------------------
 * Type Definitions
 *---------------------------------------------------------*/
enum states {
    S0_ALL_OFF,      // All LEDs off
    S1_LED1_BLINK,   // LED 1 blink at 4 Hz
    S2_LED13_ON,     // LED 1 and 3 on, LED 2 and 4 off
    S3_LED24_ON,     // LED 1 and 3 off, LED 2 and 4 on
    S4_ALL_BLINK     // All LEDs blink at 16 Hz
};

typedef struct {
    struct smf_ctx ctx;
    uint32_t count;           // General purpose counter
    bool button_pressed[4];   // Track button states
    bool led_state;           // Current LED on/off state for blinking
} state_machine_t;

/*-------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------*/
static state_machine_t sm_object;
static const struct smf_state states[];

/*-------------------------------------------------------
 * Helper Functions
 *---------------------------------------------------------*/
static void set_all_leds(bool state) {
    LED_set(LED0, state ? LED_ON : LED_OFF);
    LED_set(LED1, state ? LED_ON : LED_OFF);
    LED_set(LED2, state ? LED_ON : LED_OFF);
    LED_set(LED3, state ? LED_ON : LED_OFF);
}

/*-------------------------------------------------------
 * State S0 - All LEDs OFF
 *---------------------------------------------------------*/
static void s0_entry(void *o) {
    set_all_leds(false);
    sm_object.count = 0;
}

static enum smf_state_result s0_run(void *o) {
    // Transition to S1 on Button 1
    if (sm_object.button_pressed[BUTTON_1]) {
        sm_object.button_pressed[BUTTON_1] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S1_LED1_BLINK]);
    }
    // Transition to S4 on Button 4
    else if (sm_object.button_pressed[BUTTON_4]) {
        sm_object.button_pressed[BUTTON_4] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S4_ALL_BLINK]);
    }
    
    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State S1 - LED 1 Blink at 4 Hz
 *---------------------------------------------------------*/
static void s1_entry(void *o) {
    sm_object.count = 0;
    sm_object.led_state = false;
    set_all_leds(false);
}

static enum smf_state_result s1_run(void *o) {
    // 4 Hz = 250ms period (125ms on, 125ms off)
    // At 1ms sleep: 125 counts per half period
    if (sm_object.count >= 125) {
        sm_object.count = 0;
        sm_object.led_state = !sm_object.led_state;
        LED_set(LED0, sm_object.led_state ? LED_ON : LED_OFF);
    } else {
        sm_object.count++;
    }
    
    // Transition to S0 on Button 4
    if (sm_object.button_pressed[BUTTON_4]) {
        sm_object.button_pressed[BUTTON_4] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S0_ALL_OFF]);
    }
    // Transition to S2 on Button 2
    else if (sm_object.button_pressed[BUTTON_2]) {
        sm_object.button_pressed[BUTTON_2] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S2_LED13_ON]);
    }
    
    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State S2 - LED 1 and 3 ON, LED 2 and 4 OFF
 *---------------------------------------------------------*/
static void s2_entry(void *o) {
    LED_set(LED0, LED_ON);   // LED 1 on
    LED_set(LED1, LED_OFF);  // LED 2 off
    LED_set(LED2, LED_ON);   // LED 3 on
    LED_set(LED3, LED_OFF);  // LED 4 off
    sm_object.count = 0;
}

static enum smf_state_result s2_run(void *o) {
    sm_object.count++;
    
    // Automatic transition to S3 after 2 seconds (2000ms)
    if (sm_object.count >= 2000) {
        sm_object.count = 0;
        smf_set_state(SMF_CTX(&sm_object), &states[S3_LED24_ON]);
    }
    
    // Transition to S0 on Button 4
    if (sm_object.button_pressed[BUTTON_4]) {
        sm_object.button_pressed[BUTTON_4] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S0_ALL_OFF]);
    }
    
    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State S3 - LED 1 and 3 OFF, LED 2 and 4 ON
 *---------------------------------------------------------*/
static void s3_entry(void *o) {
    LED_set(LED0, LED_OFF);  // LED 1 off
    LED_set(LED1, LED_ON);   // LED 2 on
    LED_set(LED2, LED_OFF);  // LED 3 off
    LED_set(LED3, LED_ON);   // LED 4 on
    sm_object.count = 0;
}

static enum smf_state_result s3_run(void *o) {
    sm_object.count++;
    
    // Automatic transition to S2 after 1 second (1000ms)
    if (sm_object.count >= 1000) {
        sm_object.count = 0;
        smf_set_state(SMF_CTX(&sm_object), &states[S2_LED13_ON]);
    }
    
    // Transition to S0 on Button 4
    if (sm_object.button_pressed[BUTTON_4]) {
        sm_object.button_pressed[BUTTON_4] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S0_ALL_OFF]);
    }
    
    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State S4 - All LEDs Blink at 16 Hz
 *---------------------------------------------------------*/
static void s4_entry(void *o) {
    sm_object.count = 0;
    sm_object.led_state = false;
    set_all_leds(false);
}

static enum smf_state_result s4_run(void *o) {
    // 16 Hz = 62.5ms period (31.25ms on, 31.25ms off)
    // At 1ms sleep: ~31 counts per half period
    if (sm_object.count >= 31) {
        sm_object.count = 0;
        sm_object.led_state = !sm_object.led_state;
        set_all_leds(sm_object.led_state);
    } else {
        sm_object.count++;
    }
    
    // Transition to S0 on Button 4
    if (sm_object.button_pressed[BUTTON_4]) {
        sm_object.button_pressed[BUTTON_4] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S0_ALL_OFF]);
    }
    // Transition to S1 on Button 3
    else if (sm_object.button_pressed[BUTTON_3]) {
        sm_object.button_pressed[BUTTON_3] = false;
        smf_set_state(SMF_CTX(&sm_object), &states[S1_LED1_BLINK]);
    }
    
    return SMF_EVENT_HANDLED;
}

/*-------------------------------------------------------
 * State Table
 *---------------------------------------------------------*/
static const struct smf_state states[] = {
    [S0_ALL_OFF]    = SMF_CREATE_STATE(s0_entry, s0_run, NULL, NULL, NULL),
    [S1_LED1_BLINK] = SMF_CREATE_STATE(s1_entry, s1_run, NULL, NULL, NULL),
    [S2_LED13_ON]   = SMF_CREATE_STATE(s2_entry, s2_run, NULL, NULL, NULL),
    [S3_LED24_ON]   = SMF_CREATE_STATE(s3_entry, s3_run, NULL, NULL, NULL),
    [S4_ALL_BLINK]  = SMF_CREATE_STATE(s4_entry, s4_run, NULL, NULL, NULL),
};

/*-------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------*/
void state_machine_init(void) {
    sm_object.count = 0;
    sm_object.led_state = false;
    
    // Initialize button states
    for (int i = 0; i < 4; i++) {
        sm_object.button_pressed[i] = false;
    }
    
    // Start in S0 (all LEDs off)
    smf_set_initial(SMF_CTX(&sm_object), &states[S0_ALL_OFF]);
}

int state_machine_run(void) {
    return smf_run_state(SMF_CTX(&sm_object));
}

void state_machine_set_button(button_t button, bool pressed) {
    if (button < 4) {
        sm_object.button_pressed[button] = pressed;
    }
}