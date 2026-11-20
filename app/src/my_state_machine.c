/**
*   @file my_state_machine.c
*/
#include "my_state_machine.h"

#include <zephyr/kernel.h>
#include <zephyr/smf.h>

#include "BTN.h"
#include "LED.h"

static void led_on_state_entry(void* o);
static enum smf_state_result led_on_state_run(void* o);
static void led_off_state_entry(void* o);
static enum smf_state_result led_off_state_run(void* o);

enum led_state_machine_states {
    LED_ON_STATE,
    LED_OFF_STATE
};

typedef struct {
    struct smf_ctx ctx;

    uint16_t events;
} led_state_object_t;

static const struct smf_state led_states[] = {
    [LED_ON_STATE] = SMF_CREATE_STATE(led_on_state_entry, led_on_state_run, NULL, NULL, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, NULL, NULL, NULL)
};

static led_state_object_t led_state_object;

void state_machine_init() {
    led_state_object.events = 0;
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
}

int state_machine_run() {
    led_state_object.events = k_event_wait_safe(&btn_event, BTN_ANY_EVENT, false, K_FOREVER);
    return(smf_run_state(SMF_CTX(&led_state_object)));
}

static void led_on_state_entry(void* o) {
    LED_set(LED0, LED_ON);
}

static enum smf_state_result led_on_state_run(void* o) {
    if (led_state_object.events & BTN0_EVENT) {
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
    }
    return SMF_EVENT_HANDLED;
}

static void led_off_state_entry(void* o) {
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result led_off_state_run(void* o) {
    if (led_state_object.events & BTN1_EVENT) {
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    }

    return SMF_EVENT_HANDLED;
}