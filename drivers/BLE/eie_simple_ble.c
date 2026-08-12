/**
 * @file eie_simple_ble.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-07-10
 *
 * @copyright Copyright (c) 2026
 *
 */

/*
 * Includes
 */
#include "eie_simple_ble.h"

#include <errno.h>
#include <pb.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/smf.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/dlist.h>

/*
 * Macros
 */
#define EIE_SERVICE_UUID \
  BT_UUID_128_ENCODE(0xdc95d873, 0x598d, 0x48a2, 0xabf5, 0x2b056080b8ed)

#define EIE_SERVICE_NOTIFY_CHARACTERISTIC_UUID \
  BT_UUID_128_ENCODE(0xdc95d873, 0x598d, 0x48a2, 0xabf5, 0x2b056080b8ee)

#define EIE_SERVICE_WRITE_CHARACTERISTIC_UUID \
  BT_UUID_128_ENCODE(0xdc95d873, 0x598d, 0x48a2, 0xabf5, 0x2b056080b8ef)

/*
 * Prototypes
 */
static void bluetooth_ready_cb(int err);

static void connected_cb(struct bt_conn *conn, uint8_t err);

static void disconnected_cb(struct bt_conn *conn, uint8_t reason);

static ssize_t write_char_cb(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr, const void *buf,
                             uint16_t len, uint16_t offset, uint8_t flags);

static void cccd_change_cb(const struct bt_gatt_attr *attr, uint16_t value);

static void eie_ble_main(void *, void *, void *);

/*
 * Types
 */
struct ble_thread {
  struct k_thread thread;
  k_tid_t id;
};

enum ble_events {
  CONNECTED_BIT,
  DISCONNECTED_BIT,
  CCCD_CHANGE_BIT,
  GATT_WRITE_BIT,

  CONNECTED = BIT(CONNECTED_BIT),
  DISCONNECTED = BIT(DISCONNECTED_BIT),
  CCCD_CHANGE = BIT(CCCD_CHANGE_BIT),
  GATT_WRITE = BIT(GATT_WRITE_BIT)
};

/*
 * Local Variables
 */
static const struct bt_uuid_128 eie_service_uuid =
    BT_UUID_INIT_128(EIE_SERVICE_UUID);
static const struct bt_uuid_128 notify_characteristic_uuid =
    BT_UUID_INIT_128(EIE_SERVICE_NOTIFY_CHARACTERISTIC_UUID);
static const struct bt_uuid_128 write_characteristic_uuid =
    BT_UUID_INIT_128(EIE_SERVICE_WRITE_CHARACTERISTIC_UUID);

static const struct bt_data eie_service_adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, EIE_SERVICE_UUID),
};

static uint8_t write_data[512];

BT_GATT_SERVICE_DEFINE(
    eie_service,  // Name of the struct that will store the config for
                  // this service
    BT_GATT_PRIMARY_SERVICE(&eie_service_uuid),  // Setting the service UUID

    // Now to define the characteristic:
    BT_GATT_CHARACTERISTIC(
        &notify_characteristic_uuid.uuid,  // Setting the characteristic UUID
        BT_GATT_CHRC_NOTIFY,               // Possible operations
        BT_GATT_PERM_READ,  // Permissions that connecting devices have
        NULL,               // Callback for when this characteristic is
                            // read from
        NULL,               // Callback for when this characteristic is
                            // written to
        NULL                // Initial data stored in this
                            // characteristic
        ),
    BT_GATT_CCC(  // Client characteristic configuration for the above custom
                  // characteristic
        cccd_change_cb,  // Callback for when this characteristic is changed
        BT_GATT_PERM_READ |
            BT_GATT_PERM_WRITE  // Permissions that connecting devices have
        ),
    BT_GATT_CHARACTERISTIC(&write_characteristic_uuid.uuid, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, write_char_cb, write_data)

    // End of service definition
);

BT_CONN_CB_DEFINE(conn_callbacks) = {.connected = connected_cb,
                                     .disconnected = disconnected_cb};

static struct eie_ble_cb callbacks_ = {NULL, NULL};

static struct ble_thread ble_thread = {};
/* User defined object */
K_THREAD_STACK_DEFINE(ble_thread_stack, 512);
static struct k_event ble_event;

int eie_ble_startup(struct eie_ble_cb const *callbacks) {
  if (callbacks == NULL) {
    return -EINVAL;
  }
  callbacks_ = *callbacks;

  ble_thread.id =
      k_thread_create(&ble_thread.thread, ble_thread_stack,
                      K_THREAD_STACK_SIZEOF(ble_thread_stack), eie_ble_main,
                      NULL, NULL, NULL, 1, 0, K_NO_WAIT);
#if CONFIG_THREAD_NAME
  k_thread_name_set(ble_thread.id, "BLE Thread");
#endif

  return bt_enable(bluetooth_ready_cb);
}

int eie_ble_start_advertise(const char *device_name) {
  struct bt_data eie_service_scan_response[] = {
      {.type = BT_DATA_NAME_COMPLETE,
       .data_len = strlen(device_name),
       .data = device_name}};

  bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, eie_service_adv_data,
                  ARRAY_SIZE(eie_service_adv_data), eie_service_scan_response,
                  ARRAY_SIZE(eie_service_scan_response));
  return 0;
}

int eie_ble_stop_advertise(void) { return bt_le_adv_stop(); }

static void connected_cb(__maybe_unused struct bt_conn *conn,
                         __maybe_unused uint8_t err) {
  struct bt_conn_info info;
  bt_conn_get_info(conn, &info);
  uint16_t handle;
  bt_hci_get_conn_handle(conn, &handle);
  printk("info->id %d, handle %u", info.id, handle);
  printk("Connected %u\n", err);
  if (callbacks_.event_cb) {
    callbacks_.event_cb(conn, EIE_CONNECTED);
  }

  //   bt_conn_le_phy_update(struct bt_conn *conn, const struct
  //   bt_conn_le_phy_param *param) bt_gatt_exchange_mtu(struct bt_conn *conn,
  //   struct bt_gatt_exchange_params *params)
}

static void cccd_change_cb(const struct bt_gatt_attr *attr, uint16_t value) {
  printk("CCCD changed %u\n", value);
}

static void disconnected_cb(__maybe_unused struct bt_conn *conn,
                            __maybe_unused uint8_t reason) {
  printk("Disconnected %u\n", reason);
}

static void bluetooth_ready_cb(int err) { printk("BLE ready %d\n", err); }

static ssize_t write_char_cb(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr, const void *buf,
                             uint16_t len, uint16_t offset, uint8_t flags) {
  printk("Write %u %u", len, offset);
  uint8_t *dest = (uint8_t *)attr->user_data + offset;
  memcpy(dest, buf, len);

  struct eie_message_data message = {len, sys_le16_to_cpu(*dest), dest + 2};
  if (callbacks_.message_cb) {
    callbacks_.message_cb(conn, &message);
  }

  return len;
}

static void eie_ble_main(void *p1 __attribute__((unused)),
                         void *p2 __attribute__((unused)),
                         void *p3 __attribute__((unused))) {
  k_event_init(&ble_event);

  while (1) {
    uint32_t events;

    events = k_event_wait(&ble_event, 0xFFF, false, K_FOREVER);
  }
}
