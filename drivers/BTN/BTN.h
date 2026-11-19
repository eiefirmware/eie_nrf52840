/*
Header to define button interface
*/

#ifndef BTN_H
#define BTN_H

#include <stdbool.h>
#include <zephyr/kernel.h>

/* ----------------------------------------------------------------------------
                                    TYPES
---------------------------------------------------------------------------- */
typedef enum btn_id_t {
  BTN0 = 0,
  BTN1,
  BTN2,
  BTN3,
  NUM_BTNS,
} btn_id;

typedef enum btn_event_id_t {
  BTN0_EVENT = BIT(BTN0),
  BTN1_EVENT = BIT(BTN1),
  BTN2_EVENT = BIT(BTN2),
  BTN3_EVENT = BIT(BTN3),
  BTN_ANY_EVENT = BTN0_EVENT | BTN1_EVENT | BTN2_EVENT | BTN3_EVENT,
} btn_event_id;

/*-----------------------------------------------------------------------------
                                Global Variables
-----------------------------------------------------------------------------*/
extern struct k_event btn_event;

/* ----------------------------------------------------------------------------
                              Public Functions
---------------------------------------------------------------------------- */
int BTN_init();

bool BTN_is_pressed(btn_id btn);

bool BTN_check_clear_pressed(btn_id btn);

bool BTN_check_pressed(btn_id btn);

void BTN_clear_pressed(btn_id btn);

#endif