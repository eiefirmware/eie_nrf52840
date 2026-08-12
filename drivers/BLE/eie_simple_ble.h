/**
 * @file eie_simple_ble.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-07-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdint.h>
#include <sys/cdefs.h>
#include <zephyr/bluetooth/bluetooth.h>

enum eie_ble_event {
  EIE_CONNECTED,
  EIE_DISCONNECTED,
};

struct eie_message_data {
  size_t length;
  uint16_t message_id;
  uint8_t* data;
};

typedef void (*eie_ble_event_cb)(struct bt_conn*, enum eie_ble_event);

typedef void (*eie_ble_message_received_cb)(struct bt_conn*,
                                            struct eie_message_data*);

struct eie_ble_cb {
  eie_ble_event_cb event_cb;
  eie_ble_message_received_cb message_cb;
};

int eie_ble_startup(struct eie_ble_cb const* callbacks);

/**
 * @brief
 *
 */
int eie_ble_start_advertise(const char* device_name);

int eie_ble_stop_advertise(void);
