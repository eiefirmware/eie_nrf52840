/**
 * @file main.c
 */

#include <inttypes.h>
#include <pb_decode.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "eie_simple_ble.h"

#define SLEEP_MS 1

void eie_event(__maybe_unused struct bt_conn* conn, enum eie_ble_event event) {
  printk("event %u\n", event);
}

void message_received(__maybe_unused struct bt_conn* conn,
                      struct eie_message_data* message) {
  printk("id %u\n", message->message_id);
}

int main(void) {
  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }
  if (0 > eie_ble_startup(&(struct eie_ble_cb){eie_event, message_received})) {
    return 0;
  }
  eie_ble_connect("Test EIE Device");
  LED_blink(LED1, LED_1HZ);
  while (1) {
    k_msleep(SLEEP_MS);
  }
  return 0;
}
