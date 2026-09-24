/**
 * @file linked_list.c
 *
 */

#include "linked_list.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <zephyr/sys/printk.h>

typedef struct node {
  uint32_t data;
  struct node* next;
  struct node* prev;
} node_t;

typedef struct linked_list {
  node_t* head;
  node_t* cursor;
} linked_list_t;

linked_list_t* ll_create_new(void) {
  linked_list_t* linked_list = malloc(sizeof(linked_list_t));
  assert(linked_list);

  linked_list->head = NULL;
  linked_list->cursor = NULL;

  return linked_list;
}

void ll_destroy(linked_list_t* linked_list) {
  if (linked_list == NULL) {
    return;
  }
  // Find all nodes in the linked list and free them
  node_t* node = linked_list->head;
  while (node != NULL) {
    node_t* next_node = node->next;
    free(node);

    node = next_node;
  }

  // Free the full linked list object
  free(linked_list);
}

void ll_append_data(linked_list_t* linked_list, int32_t data) {
  if (linked_list == NULL) {
    return;
  }
  node_t* new_node = malloc(sizeof(node_t));
  assert(new_node);
  new_node->data = data;
  new_node->next = NULL;

  // If this is a new linked list update the head and cursor
  if (linked_list->head == NULL) {
    linked_list->head = new_node;
    linked_list->cursor = new_node;
    new_node->prev = NULL;
    return;
  }

  // Seek the current tail
  node_t* node;
  for (node = linked_list->head; node->next != NULL; node = node->next);

  // Update the tail with the new node
  node->next = new_node;
  new_node->prev = node;
}

void ll_add_data_at_cursor(linked_list_t* linked_list, int32_t data) {
  if (linked_list == NULL) {
    return;
  } else if (linked_list->cursor == NULL) {
    // If the cursor is null the list hasn't been initialized. Append
    // data already handles initialization so just use it.
    ll_append_data(linked_list, data);
    return;
  }

  node_t* new_node = malloc(sizeof(node_t));
  assert(new_node);
  new_node->data = data;

  // Update linkage
  if (linked_list->cursor->next != NULL) {
    linked_list->cursor->next->prev = new_node;
  }
  new_node->next = linked_list->cursor->next;
  new_node->prev = linked_list->cursor;
  linked_list->cursor->next = new_node;
};

void ll_remove_data_at_cursor(linked_list_t* linked_list) {
  // Check for null pointers. Return early if a null pointer is passed in
  if (linked_list == NULL || linked_list->cursor == NULL) {
    return;
  }
  // Create a local copy of the cursor for convenience.
  node_t* cursor = linked_list->cursor;

  // Get the elements before and after the cursor accounting for null
  node_t* before_cursor = NULL;
  if (cursor->prev != NULL) {
    before_cursor = cursor->prev;
  }
  node_t* after_cursor = NULL;
  if (cursor->next != NULL) {
    after_cursor = cursor->next;
  }

  // Update where the pointers are pointing. The element before the
  // cursor should now be pointing to the element after the cursor and
  // vice versa
  if (before_cursor != NULL) {
    // Hint something is missing here:

    // Update the cursor in the linked list to point to the element that
    // was before the cursor
    linked_list->cursor = before_cursor;
  } else {
    // If the element before the cursor was null, that means the cursor
    // was at the head so update both the cursor and head to point to
    // the element that was after the cursor.
    linked_list->cursor = after_cursor;
    linked_list->head = after_cursor;
  }
  if (after_cursor != NULL) {
    after_cursor->prev = before_cursor;
  }

  free(cursor);
}

void ll_cursor_move_forward(linked_list_t* linked_list, int n) {
  if (linked_list == NULL || linked_list->cursor == NULL) {
    return;
  }

  for (int i = 0; i < n && linked_list->cursor->next != NULL; i++) {
    linked_list->cursor = linked_list->cursor->next;
  }
}

void ll_cursor_move_back(linked_list_t* linked_list, int n) {
  if (linked_list == NULL || linked_list->cursor == NULL) {
    return;
  }

  for (int i = 0; i < n && linked_list->cursor->prev != NULL; i++) {
    linked_list->cursor = linked_list->cursor->prev;
  }
}

int32_t ll_cursor_get_data(linked_list_t* linked_list) {
  if (linked_list == NULL || linked_list->cursor == NULL) {
    return 0xFFFFFFFF;
  }

  return linked_list->cursor->data;
}

void ll_print_data(linked_list_t const* linked_list) {
  if (linked_list == NULL) {
    return;
  }
  for (node_t const* node = linked_list->head; node != NULL;
       node = node->next) {
    printk("%d ", node->data);
  }
  printk("\n");
}

void ll_print_data_reverse(linked_list_t const* linked_list) {
  if (linked_list == NULL || linked_list->head == NULL) {
    return;
  }

  node_t const* node;
  for (node = linked_list->head; node->next != NULL; node = node->next);

  for (; node != NULL; node = node->prev) {
    printk("%d ", node->data);
  }
  printk("\n");
}
