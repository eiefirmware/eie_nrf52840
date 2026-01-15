# BLE Central

[TOC]

## Introduction

Before the term break, we went over how to setup the development boards as a BLE GAP *peripheral* (and GATT *server*),
which allowed them to searched for by other devices, and present various services and characteristics for other device to read from, write to, and/or be notified from.

For completeness, this lesson will cover the other half of BLE functionality,
namely how to make the development board act as a BLE GAP *central*, (and GATT *client*).
This is useful for when we want our boards to be the one scanning for and connecting to other devices,
performing the functionality that we used nRF Connect for in the previous sessions.

There's a bit more complexity required for the GAP central role,
since now our boards will have to actively make decisions about which devices to connect to,
and then what to do with the services and characteristics presented by the connected device.
As a result, please make sure to ask leaders and those around you for clarification or help if you are having trouble with the content!

## Lesson

### Project Config

First, we'll need to enable the BLE functionality in Zephyr,
specifically for the *GAP central* role and the *GATT server* capability.
You'll need the following in your `prj.conf`:

```conf
CONFIG_BT=y
CONFIG_BT_CENTRAL=y
CONFIG_BT_GATT_CLIENT=y
CONFIG_BT_HCI_ERR_TO_STR=y
```

### Includes

Similarly, we'll need to include the headers that allow us to use the needed functions.
Add these includes to your `main.c` file:

```cpp
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>
```

### UUIDs

The last bit of config will be the UUIDs for our custom service and characteristic
on the device we'll be connecting to.
Add these static structs to your `main.c`:

```cpp
static struct bt_uuid_128 BLE_CUSTOM_SERVICE_UUID =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000001));
static struct bt_uuid_128 BLE_CUSTOM_CHARACTERISTIC_UUID =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000002));
```

### Connect/Disconnect Callbacks

In order to know when our boards connect to or disconnect from another BLE device,
we'll need to define some functions and configure them as callbacks!

These both need to have the following signature:
```cpp
static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {
  ...
}

static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t reason) {
  ...
}
```

Let's also define a global pointer to track our current connection object:
```cpp
static struct bt_conn* my_connection;
```

#### Connect Handler

Inside the **connected** handler,
be sure to first check that `err` is equal to 0,
since any other value means that the *connection has failed*!
In this case, we'll want to clear our global connection object wil `bt_conn_unref(my_connection)`
and `my_connection = NULL`.

Next, we have to make sure that the connection event we're handling is for the connection that
our board actually initiated!
The `conn` argument is a pointer to the connection object that this event is for,
and our global `my_connection` variable is tracking the connection that we actually tried to make.
If these are different, then we don't care about the incoming connection and we can ignore it.

Finally, let's see what the address of our connected device is!
We can get a string of the device's MAC address with the following:
```cpp
  char addr[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  printk("Connected: %s\n", addr);
```

#### Disconnect Handler

The disconnect handler is very similar to the connect handler.
We also want to ignore any `conn` value that is not the intended `my_connection`.
And we also always want to **unref** then **clear** our `my_connection` variable!

One new bit of information we have is the disconnect reason in the `reason` argument,
which we can turn into a human-readable string with `bt_hci_err_to_str(reason)`.
Print this out when we disconnect so that we can know why!

#### Setting the Handlers as Callbacks

Once our handlers are defined,
we just need to tell the OS to call them when devices are connected/disconnected!
Put this *outside* of any function, before `main()`:

```cpp
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = ble_on_device_connected,
    .disconnected = ble_on_device_disconnected,
};
```

### Scanning and Connecting

Once we have our connect and disconnect handlers defined and configured,
we'll have to figure out a way to get the board to actually *scan* for any devices to connect to.
The API we use for this is the following: 

```cpp
bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_on_advertisement_received)
```

Note here that we're doing what's called a "passive" scan.
This means that our board is simply listening to whatever data advertising devices are sending out.
BLE does have a mechanism where we could instead ask these advertising devices for more data,
a process called "active scanning",
but not every device supports it, and we don't need it here since we aren't filtering through
many advertising devices looking for a lot of data before trying to connect
(or not connecting at all).

Notice that there's a reference to a function called `ble_on_advertisement_received`.
This is another callback we have to define!
Note that there seems to be a lot of callbacks needed for BLE programming.
This is a natural effect of BLE events needing strict timing and/or *requiring* some amount of time in order to complete.
By using callbacks, we can configure the OS to just "tell us when you have something"
(asynchronous) as opposed to having to ask "do you have new data?" repeatedly (synchronous).
Our callback needs the following signature:

```cpp
static void ble_on_advertisement_received(const bt_addr_le_t* addr, int8_t rssi, uint8_t type,
                                          struct net_buf_simple* ad) {
  ...
}
```

First, let's add a filter to ignore any received packets that we don't carte about.
Ignore the data and return early if:

1. We're already connected to a device (`my_connection` is *not* `NULL`).
2. The advertisement type is "non-connectable" (`type` is anything other than `BT_GAP_ADV_TYPE_ADV_IND` or `BT_GAP_ADV_TYPE_ADV_DIRECT_IND`)

These filters will help us ignore packets we don't care about,
freeing up compute time and power.
The packets that get through the filter are now only from *connectable* devices that are *close to your board*.

Next, let's check the advertising device's name,
if it chose to include it (not always the case).
This means we'll need... you guessed it, another callback!
This one's pretty simple, so I'll just give it to you:

```cpp
static bool ble_get_adv_device_name_cb(struct bt_data* data, void* user_data) {
  char* name = user_data;

  if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {
    memcpy(name, data->data, data->data_len);
    name[data->data_len] = 0; // Null-terminator
    return false; // Stop parsing
  }

  return true; // Continue looking through this advertising packet
}
```

We can then get the device name from the advertising packet as follows:

```cpp
  char name[32] = {0};
  bt_data_parse(ad, ble_get_adv_device_name_cb, name);
```

Print this name to the console in this handler, as well as the device's MAC address
(using `bt_addr_le_to_str` like above).
Finally, let's check that this name is the name of our phone (which we will be configuring next).
Obviously we don't want to connect things with a bad signal strength,
so after printing the address and name out to the console,
return if the `rssi` argument is less than `-50` (yes, negative!).

Otherwise, if the name matches, stop scanning with `bt_le_scan_stop()`, then connect to the device!
This is done with the following:

```cpp
  bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &my_connection);
```
Make sure the return value of this is 0, just like `err` in the connection handler.
If the return value is anything else, that means the connection failed!

#### Starting The Scan From Main

Just like with the peripheral mode,
we need to enable BLE in the `main()` function:

```cpp
int main(void) {
  ...

  int err = bt_enable(NULL);
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return 0;
  } else {
    printk("Bluetooth initialized\n");
  }
  ...
}
```

Then just start scanning with `bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_on_advertisement_received)`!

### nRF Connect GATT Server Setup

Just like we used our phones last time to connect to the board when it was a BLE *peripheral*,
we're now going to use our phones as the peripherals, since our boards are now *centrals*.
This means that your board will now be trying to scan for and connect to your phone.

In the nRF Connect app, navigate to "Configure GATT Server",
then create a new configuration called "EiE Peripheral". 
Next, add a custom service with the same UUID as `BLE_CUSTOM_SERVICE_UUID` in your code
(e.g. `11111111-2222-3333-4444-000000000001`).
The name is optional.

Next, under this new custom service, add a new characteristic in the same fashion,
using the same UUID as `BLE_CUSTOM_SERVICE_UUID` in your code
(e.g. `11111111-2222-3333-4444-000000000002`).
**Make sure to enable the `Notify` property!**
This will allow your device to subscribe to notifications from this characteristic later in the lesson.
You should see that your custom characteristic was added,
as well as a "Client Characteristic Configuration" descriptor (sometimes called a "CCCD").
This descriptor is what allows other devices to actually subscribe to the notifications.

Finally, navigate to the "Advertiser" section of the "Devices" pane,
and create a new advertising configuration.
Give the config a display name
(just visible in the app, this is **not** the name sent in the advertising packets),
enable the `Connectable` option,
then under "Advertising data",
add the `Complete Local Name` record
(**this** is what actually tells your phone to include its name in the advertising data).

### Try It!

At this point, you should make sure that your board does what you want it to do.
Compile and flash your code,
then open up a serial monitor.
You should see your device receiving various advertising packets.

On your phone, if you then enable your new advertising configuration in nRF connect
and hold it close to your board,
you should see it appear and your board should connect!

Once this happens, now we'll have to actually try and get data from the phone,
so we'll try subscribing to notifications on the custom characteristic in the next section.

### Service Discovery

**Service discovery is very complicated, so we're going to give you the code for this.**

When you connect to a BLE device,
you need to "discover" what services and characteristics are available.
We're going to be searching for our custom service and characteristic by UUID here.

First, let's add some global variables:

```cpp
static struct bt_uuid_16 discover_uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;
```

Next, add this to the end of `ble_on_device_connected`:

```cpp
static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {
  ...

  discover_params.uuid = &BLE_CUSTOM_SERVICE_UUID.uuid;
  discover_params.func = discover_func;
  discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
  discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
  discover_params.type = BT_GATT_DISCOVER_PRIMARY;

  err = bt_gatt_discover(my_connection, &discover_params);
  if (err) {
    printk("Discover failed(err %d)\n", err);
    return;
  }
}
```

Finally, add in this function:

```cpp
static uint8_t discover_func(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                             struct bt_gatt_discover_params* params) {
  int err;

  if (!attr) {
    printk("Discover complete\n");
    (void)memset(params, 0, sizeof(*params));
    return BT_GATT_ITER_STOP;
  }

  printk("[ATTRIBUTE] handle %u\n", attr->handle);

  if (!bt_uuid_cmp(discover_params.uuid, &BLE_CUSTOM_SERVICE_UUID.uuid)) {
    discover_params.uuid = &BLE_CUSTOM_CHARACTERISTIC_UUID.uuid;
    discover_params.start_handle = attr->handle + 1;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

    err = bt_gatt_discover(conn, &discover_params);
    if (err) {
      printk("Discover failed (err %d)\n", err);
    }
  } else if (!bt_uuid_cmp(discover_params.uuid, &BLE_CUSTOM_CHARACTERISTIC_UUID.uuid)) {
    memcpy(&discover_uuid, BT_UUID_GATT_CCC, sizeof(discover_uuid));
    discover_params.uuid = &discover_uuid.uuid;
    discover_params.start_handle = attr->handle + 2;
    discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
    subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

    err = bt_gatt_discover(conn, &discover_params);
    if (err) {
      printk("Discover failed (err %d)\n", err);
    }
  } else {
    subscribe_params.notify = notify_func;
    subscribe_params.value = BT_GATT_CCC_NOTIFY;
    subscribe_params.ccc_handle = attr->handle;

    err = bt_gatt_subscribe(conn, &subscribe_params);
    if (err && err != -EALREADY) {
      printk("Subscribe failed (err %d)\n", err);
    } else {
      printk("[SUBSCRIBED]\n");
    }

    return BT_GATT_ITER_STOP;
  }

  return BT_GATT_ITER_STOP;
}
```

If you need help,
or are wondering how this works, ask a leader!
Zephyr's BLE GATT client API is pretty low-level,
making it more complicated to use than for the GATT server side.

### Subscribing to Notifications

The last thing we need is a callback to get called when we receive a notification.
This is as follows:

```cpp
static uint8_t notify_func(struct bt_conn* conn, struct bt_gatt_subscribe_params* params,
                           const void* data, uint16_t length) {
  if (!data) {
    printk("[UNSUBSCRIBED]\n");
    params->value_handle = 0U;
    return BT_GATT_ITER_STOP;
  }

  printk("[NOTIFICATION] data %p length %u\n", data, length);
  for (int i = 0; i < MIN(length, 16); i++) {
    printk(" 0x%02X", ((uint8_t*)data)[i]);
  }
  printk("\n");

  return BT_GATT_ITER_CONTINUE;
}
```

### Try It! Again!

With all of this code, you should have a board that scans for BLE devices,
finds your phone by device name,
connects to it,
subscribes to notifications on your custom characteristic in your custom service,
and prints out any data received!

Compile and flash your board again,
connect your phone and the board,
then send some notifications via the "GATT Server" section of the connection pane in nRF Connect.

If you need help,
as always,
ask a leader!

The BLE GAP client role is more complicated than the GAP peripheral role,
and not as useful for your project,
so don't worry too hard if you had difficulties!

<!--
## Extra

### Reading Characteristics

```cpp
  static struct bt_gatt_read_params read_params;
  read_params.handle_count = 1;
  read_params.single.handle = led_state_handle;
  read_params.single.offset = 0;
  read_params.func = read_func;
  bt_gatt_read(led_conn, &read_params);
```

```cpp
static uint8_t read_func(struct bt_conn *conn, uint8_t err,
                         struct bt_gatt_read_params *params, const void *data,
                         uint16_t length) {
  led_state = *((uint8_t *)data);
  printk("LED Control value: %u\n", led_state);
  k_event_set(&event, EV_STATE_READ);
  return BT_GATT_ITER_STOP;
}
```

### Writing Characteristics

```cpp
  static struct bt_gatt_write_params write_params;
  write_params.handle = led_state_handle;
  write_params.offset = 0;
  write_params.data = &led_state;
  write_params.length = 1;
  write_params.func = write_func;
  bt_gatt_write(led_conn, &write_params);
```

```cpp
static void write_func(struct bt_conn *conn, uint8_t err,
                       struct bt_gatt_write_params *params) {
  if (err) {
    printk("Write did not work: err %u\n", err);
  } else {
    printk("Turned LED %s\n", led_state ? "on" : "off");
  }
  k_event_set(&event, EV_STATE_WRITTEN);
}
```
-->