/**
 * @file main.c
 */

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "linked_list.h"

#define SLEEP_MS 1

int main(void) {
  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  linked_list_t* my_linked_list = ll_create_new();

  for (int i = 0; i < 10; i++) {
    ll_append_data(my_linked_list, i);
  }

  ll_print_data(my_linked_list);  // Expected: 0 1 2 3 4 5 6 7 8 9

  ll_cursor_move_forward(my_linked_list, 5);

  printk("data at cursor %d\n", ll_cursor_get_data(my_linked_list));

  ll_remove_data_at_cursor(my_linked_list);  // Expected 5

  ll_print_data(my_linked_list);  // Expected: 0 1 2 3 4 6 7 8 9

  ll_print_data_reverse(my_linked_list);

  ll_cursor_move_back(my_linked_list, 2);

  ll_add_data_at_cursor(my_linked_list, 55);

  ll_remove_data_at_cursor(my_linked_list);

  ll_print_data(my_linked_list);  // Expected: 0 1 2 55 4 6 7 8 9

  ll_destroy(my_linked_list);

  while (1) {
    k_msleep(SLEEP_MS);
  }
  return 0;
}
