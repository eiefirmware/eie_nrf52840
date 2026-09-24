/**
 * @file linked_list.h
 *
 * @brief Doubly linked list API used by the sample application.
 */

#include <stdint.h>

typedef struct linked_list linked_list_t;

/**
 * @brief Creates a new empty linked list.
 *
 * @return Pointer to a newly allocated linked list object.
 */
linked_list_t* ll_create_new(void);

/**
 * @brief Frees all nodes in the list and then frees the list object itself.
 *
 * @param linked_list Pointer to the list to destroy. If NULL, no action is
 * taken.
 */
void ll_destroy(linked_list_t* linked_list);

/**
 * @brief Appends data to the end of the list.
 *
 * If the list is empty, the new node becomes both the head and the cursor.
 *
 * @param linked_list Pointer to the list to modify.
 * @param data Integer value to insert at the tail of the list.
 */
void ll_append_data(linked_list_t* linked_list, int32_t data);

/**
 * @brief Inserts data immediately after the current cursor position.
 *
 * If the list is empty or the cursor is NULL, the data is appended to the end
 * of the list, which initializes the list as needed.
 *
 * @param linked_list Pointer to the list to modify.
 * @param data Integer value to insert after the current cursor node.
 */
void ll_add_data_at_cursor(linked_list_t* linked_list, int32_t data);

/**
 * @brief Removes the node currently pointed to by the cursor.
 *
 * The cursor is advanced to the next node if one exists; otherwise it moves to
 * the previous node. If the list becomes empty, the head and cursor are set to
 * NULL.
 *
 * @param linked_list Pointer to the list to modify.
 */
void ll_remove_data_at_cursor(linked_list_t* linked_list);

/**
 * @brief Returns the data stored in the current cursor node.
 *
 * @param linked_list Pointer to the list being queried.
 * @return The data value at the cursor, or 0xFFFFFFFF if the list or cursor is
 * NULL.
 */
int32_t ll_cursor_get_data(linked_list_t* linked_list);

/**
 * @brief Moves the cursor forward by up to n elements.
 *
 * The cursor stops when it reaches the tail or when n steps have been taken.
 *
 * @param linked_list Pointer to the list to modify.
 * @param n Maximum number of nodes to move forward.
 */
void ll_cursor_move_forward(linked_list_t* linked_list, int n);

/**
 * @brief Moves the cursor backward by up to n elements.
 *
 * The cursor stops when it reaches the head or when n steps have been taken.
 *
 * @param linked_list Pointer to the list to modify.
 * @param n Maximum number of nodes to move backward.
 */
void ll_cursor_move_back(linked_list_t* linked_list, int n);

/**
 * @brief Prints the list in forward order from head to tail.
 *
 * @param linked_list Pointer to the list to print. If NULL, no output is
 * produced.
 */
void ll_print_data(linked_list_t const* linked_list);

/**
 * @brief Prints the list in reverse order from tail to head.
 *
 * @param linked_list Pointer to the list to print. If NULL, no output is
 * produced.
 */
void ll_print_data_reverse(linked_list_t const* linked_list);
