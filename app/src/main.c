
/**
 * @file main.c
 */

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1
#define PASSWORD_MAX_LENGTH 10

typedef enum {
    ENTRY_MODE,
    LOCKED_MODE,
    WAITING_MODE
} program_state;


int startup_sequence(void);
int password_input(int *password, int max_size);
int password_entry(const int *saved_password,
                   const int *entered_password,
                   int saved_size,
                   int entered_size);
int entry_mode(int *saved_password);
void locked_mode(void);
void waiting_mode(void);


int main(void) {

    if (0 > BTN_init()) {
        return -1;
    }

    if (0 > LED_init()) {
        return -1;
    }

   
    int saved_password[PASSWORD_MAX_LENGTH] = {0, 1, 2};
    int saved_size = 3;

   
    int enter_requested = startup_sequence();

    
    if (enter_requested) {
        int new_size = entry_mode(saved_password);

        if (new_size > 0) {
            saved_size = new_size;
        }
    }

    program_state state = LOCKED_MODE;

    while (1) {

        switch (state) {

        case LOCKED_MODE: {
            LED_set(LED0, LED_ON);

            int entered_password[PASSWORD_MAX_LENGTH];

            int entered_size = password_input(
                entered_password, PASSWORD_MAX_LENGTH);

            if (password_entry(saved_password,
                               entered_password,
                               saved_size,
                               entered_size)) {

                printk("Correct!\n");

            } else {

                printk("Incorrect!\n");
            }

            state = WAITING_MODE;
            break;
        }

        case WAITING_MODE:
            LED_set(LED0, LED_OFF);
            waiting_mode();
            state = LOCKED_MODE;
            break;

        default:
            state = LOCKED_MODE;
            break;
        }

    }

    return 0;
}



int startup_sequence(void) {

    int enter_requested = 0;

    LED_set(LED3, LED_ON);

    int64_t start_time = k_uptime_get();

    while ((k_uptime_get() - start_time) < 3000) {

        if (BTN_check_clear_pressed(BTN3)) {
            enter_requested = 1;
        }

        k_msleep(SLEEP_MS);
    }

    LED_set(LED3, LED_OFF);

    return enter_requested;
}



int password_input(int *password, int max_size) {

    int size = 0;
    int end = 0;

    while (!end) {

        if (BTN_check_clear_pressed(BTN0)) {

            if (size < max_size) {
                password[size] = 0;
                size++;
            }

        } else if (BTN_check_clear_pressed(BTN1)) {

            if (size < max_size) {
                password[size] = 1;
                size++;
            }

        } else if (BTN_check_clear_pressed(BTN2)) {

            if (size < max_size) {
                password[size] = 2;
                size++;
            }

        } else if (BTN_check_clear_pressed(BTN3)) {

            end = 1;
        }

        k_msleep(SLEEP_MS);
    }

    return size;
}



int password_entry(const int *saved_password,
                   const int *entered_password,
                   int saved_size,
                   int entered_size) {

    if (saved_size != entered_size) {
        return 0;
    }

    for (int i = 0; i < saved_size; i++) {

        if (saved_password[i] != entered_password[i]) {
            return 0;
        }
    }

    return 1;
}



int entry_mode(int *saved_password) {

    LED_set(LED3, LED_ON);

    printk("Enter new password, then press BTN3 to save.\n");

    int size = password_input(saved_password,
                              PASSWORD_MAX_LENGTH);

    if (size > 0) {
        printk("Password saved!\n");
    } else {
        printk("Empty password. Previous password retained.\n");
    }

    LED_set(LED3, LED_OFF);

    return size;
}



void waiting_mode(void) {


    BTN_clear_pressed(BTN0);
    BTN_clear_pressed(BTN1);
    BTN_clear_pressed(BTN2);
    BTN_clear_pressed(BTN3);

    while (1) {

        if (BTN_check_clear_pressed(BTN0) ||
            BTN_check_clear_pressed(BTN1) ||
            BTN_check_clear_pressed(BTN2) ||
            BTN_check_clear_pressed(BTN3)) {

            return;
        }

        k_msleep(SLEEP_MS);
    }
}
