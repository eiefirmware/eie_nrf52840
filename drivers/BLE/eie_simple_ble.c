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
#include <stdint.h>
#include <stdlib.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
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

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf);

static bool parse_adv_packet(struct bt_data *data, void *user_data);

static ssize_t write_char_cb(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr, const void *buf,
                             uint16_t len, uint16_t offset, uint8_t flags);

static void cccd_change_cb(const struct bt_gatt_attr *attr, uint16_t value);

static void scan_state_start(void *obj);

static enum smf_state_result scan_state_run(void *obj);

static void scan_state_exit(void *obj);

static void connecting_state_start(void *obj);

static enum smf_state_result connecting_state_run(void *obj);

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
  ADV_IND_BIT,
  SCAN_RSP_BIT,
  API_EVENT_BIT,

  CONNECTED = BIT(CONNECTED_BIT),
  DISCONNECTED = BIT(DISCONNECTED_BIT),
  CCCD_CHANGE = BIT(CCCD_CHANGE_BIT),
  GATT_WRITE = BIT(GATT_WRITE_BIT),
  ADV_IND = BIT(ADV_IND_BIT),
  SCAN_RSP = BIT(SCAN_RSP_BIT),
  API_EVENT = BIT(API_EVENT_BIT),
};

struct ble_state_object {
  struct smf_ctx ctx;

  struct k_event smf_event;
  enum ble_events event;
  struct k_fifo advertising_data;
  bt_addr_le_t peer_addr;
  struct bt_conn *conn;

  char name[BT_GAP_ADV_MAX_ADV_DATA_LEN];
};

enum ble_states {
  STATE_IDLE,
  STATE_SCANNING,
  STATE_ADVERTISING,
  STATE_CONNECTING,
  STATE_CONNECTED,
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

static struct ble_state_object state_obj;
static const struct smf_state ble_states[] = {
    [STATE_IDLE] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [STATE_SCANNING] = SMF_CREATE_STATE(scan_state_start, scan_state_run,
                                        scan_state_exit, NULL, NULL),
    [STATE_ADVERTISING] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [STATE_CONNECTING] = SMF_CREATE_STATE(
        connecting_state_start, connecting_state_run, NULL, NULL, NULL),
    [STATE_CONNECTED] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
};

/***********************************************************************
 * PUBLIC API DEFINITIONS
 **********************************************************************/
int eie_ble_startup(struct eie_ble_cb const *callbacks) {
  if (callbacks == NULL) {
    return -EINVAL;
  }
  callbacks_ = *callbacks;

  k_event_init(&state_obj.smf_event);
  smf_set_initial(SMF_CTX(&state_obj), &ble_states[STATE_IDLE]);
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

int eie_ble_connect(char const *device_name) {
  printk("Attempt connect\n");
  if (device_name == NULL ||
      strlen(device_name) > BT_GAP_ADV_MAX_ADV_DATA_LEN - 1) {
    return -EINVAL;
  } else if (SMF_CTX(&state_obj)->current) {
    strncpy(state_obj.name, device_name, BT_GAP_ADV_MAX_ADV_DATA_LEN - 1);
  }

  smf_set_state(SMF_CTX(&state_obj), &ble_states[STATE_SCANNING]);
  k_event_post(&state_obj.smf_event, API_EVENT);

  return 0;
}

/***********************************************************************
 * ZEPHYR BLE CALLBACK DEFINITIONS
 **********************************************************************/

static void connected_cb(__maybe_unused struct bt_conn *conn,
                         __maybe_unused uint8_t err) {
  struct bt_conn_info info;
  bt_conn_get_info(conn, &info);
  uint16_t handle;
  bt_hci_get_conn_handle(conn, &handle);
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

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf) {
  if (adv_type != BT_GAP_ADV_TYPE_ADV_IND &&
      adv_type != BT_GAP_ADV_TYPE_SCAN_RSP) {
    return;
  }
  // The net_buf_simple is intended for stack allocation. Hack to make it work
  // on the heap
  struct net_buf_simple *adv_data =
      calloc(sizeof(struct net_buf_simple) + buf->len + sizeof(bt_addr_le_t) +
                 sizeof(uint8_t),
             1);
  adv_data->size = buf->len + sizeof(bt_addr_le_t) + sizeof(uint8_t);
  net_buf_simple_init(adv_data, 0);

  // Copy the received advertising data into the net buffer.
  net_buf_simple_add_u8(adv_data, adv_type);
  net_buf_simple_add_mem(adv_data, addr, sizeof(bt_addr_le_t));
  net_buf_simple_add_mem(adv_data, buf->data, buf->len);

  struct {
    void *_;
    void *my_data;
  } *fifo_data = malloc(sizeof(*fifo_data));
  fifo_data->my_data = adv_data;
  k_fifo_put(&state_obj.advertising_data, fifo_data);

  switch (adv_type) {
    case BT_GAP_ADV_TYPE_ADV_IND: {
      k_event_post(&state_obj.smf_event, ADV_IND);
    } break;

    case BT_GAP_ADV_TYPE_SCAN_RSP: {
      k_event_post(&state_obj.smf_event, SCAN_RSP);
    } break;
    default:
      break;
  }
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

/***********************************************************************
 * STATE MACHINE AND HELPER FUNCTIONS
 **********************************************************************/
static void scan_state_start(void *obj) {
  printk("Start Scan\n");
  struct ble_state_object *s = (struct ble_state_object *)obj;

  k_fifo_init(&s->advertising_data);
  // k_event_clear(&s->smf_event, s->event);

  struct bt_le_scan_param scan_param = {
      .type = BT_LE_SCAN_TYPE_ACTIVE,
      .options = BT_LE_SCAN_OPT_CODED,
      .interval = BT_GAP_SCAN_FAST_INTERVAL,
      .window = BT_GAP_SCAN_FAST_WINDOW,
      .timeout = 1000  // 10 seconds
  };

  bt_le_scan_start(&scan_param, scan_cb);
}

// static void free_net_ptr(struct net_buf_simple **ptr) { free(*ptr); }

static enum smf_state_result scan_state_run(void *obj) {
  printk("Run Scan\n");
  struct ble_state_object *s = (struct ble_state_object *)obj;
  static bt_addr_le_t cached_addr;

  while (!k_fifo_is_empty(&s->advertising_data)) {
    // Get the advertising data out of the fifo
    struct {
      void *_;
      void *my_data;
    } *fifo_data = k_fifo_get(&s->advertising_data, K_FOREVER);
    struct net_buf_simple *adv_data = fifo_data->my_data;

    uint8_t adv_type = net_buf_simple_pull_u8(adv_data);
    bt_addr_le_t *addr =
        net_buf_simple_pull_mem(adv_data, sizeof(bt_addr_le_t));

    // If the advertising data length is 3 it likely only contains flags and we
    // can ignore it
    if (adv_type == BT_GAP_ADV_TYPE_ADV_IND && adv_data->len > 3) {
      bt_data_parse(adv_data, parse_adv_packet, addr);
      // bt_data_parse keeps parsing data until either parse_adv_packet returns
      // false. If data == 0 at the end that means the packet was valid.
      // Cache the address
      if (adv_data->len == 0) {
        printk("Cached addr\n");
        cached_addr = *addr;
      }
    }
    if (adv_type == BT_GAP_ADV_TYPE_SCAN_RSP) {
      // Check if the address matched our previous address
      if (bt_addr_eq(&cached_addr.a, &addr->a)) {
        bt_data_parse(adv_data, parse_adv_packet, addr);
        // bt_data_parse keeps parsing data until either parse_adv_packet
        // returns false. If data == 0 at the end that means the packet was
        // valid. This is the thing we want to connect to.
        if (adv_data->len == 0) {
          s->peer_addr = *addr;
          printk("Found Device\n");
          smf_set_state(SMF_CTX(s), &ble_states[STATE_CONNECTING]);
          free(adv_data);
          free(fifo_data);
          break;
        }
      }
    }
    free(adv_data);
    free(fifo_data);
  }

  return SMF_EVENT_HANDLED;
}

static void scan_state_exit(void *obj) {
  printk("Exit scan\n");
  bt_le_scan_stop();
}

static void connecting_state_start(void *obj) {
  printk("Start connecting\n");
  struct ble_state_object *s = (struct ble_state_object *)obj;
  bt_conn_le_create(&s->peer_addr, BT_CONN_LE_CREATE_CONN,
                    BT_LE_CONN_PARAM_DEFAULT, &s->conn);
}

static enum smf_state_result connecting_state_run(void *obj) {
  struct ble_state_object *s = (struct ble_state_object *)obj;
  if (s->event & CONNECTED) {
    smf_set_state(SMF_CTX(s), &ble_states[CONNECTED]);
  }
  return SMF_EVENT_HANDLED;
}

static bool parse_adv_packet(struct bt_data *data,
                             __maybe_unused void *user_data) {
  switch (data->type) {
    case BT_DATA_FLAGS:
      // Check that flags match what we expect
      if (data->data_len == 0) {
        // We got a thing with a 0 length flags.
        // Something is weird here
        return false;
      }
      uint8_t flags = *data->data;

      const uint8_t expected_flags = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;

      // Check that the flags at least have our expected flags
      return (flags & expected_flags) == expected_flags;
    case BT_DATA_NAME_COMPLETE: {
      if (data->data_len == 0) {
        // We got a thing with a 0 length name.
        // Something is weird here
        return false;
      }

      // Copy the name to a buffer and make sure it is null terminated.
      // Strings sent over the air in BLE are normally not null
      // terminated.
      char tmp_name[BT_GAP_ADV_MAX_ADV_DATA_LEN] = {0};
      memcpy(tmp_name, data->data, data->data_len);
      tmp_name[data->data_len] = '\0';

      if (0 == strcmp(tmp_name, state_obj.name)) {
        return true;
      }
      return false;
    }
    case BT_DATA_UUID128_ALL: {
      if (data->data_len != BT_UUID_SIZE_128) {
        // Data in the 128 bit UUIDs is not the correct length.
        // Something is weird here
        return false;
      }
      struct bt_uuid_128 uuid_128 = {
          .uuid =
              {
                  .type = BT_UUID_TYPE_128,
              },
      };
      bt_uuid_create(&uuid_128.uuid, data->data, BT_UUID_SIZE_128);
      if (0 ==
          bt_uuid_cmp((struct bt_uuid *)&eie_service_uuid, &uuid_128.uuid)) {
        return true;
      }
      return false;
    };

    default:
      // There is something we don't expect in the AD data. Return false
      return false;
  }
}

static void eie_ble_main(void *p1 __attribute__((unused)),
                         void *p2 __attribute__((unused)),
                         void *p3 __attribute__((unused))) {
  while (1) {
    state_obj.event =
        k_event_wait(&state_obj.smf_event, 0xFFF, false, K_FOREVER);

    printk("ble events: %u\n", state_obj.event);
    int ret = smf_run_state(SMF_CTX(&state_obj));
    k_event_clear(&state_obj.smf_event, state_obj.event);
    if (ret) {
      break;
    }
  }
}
