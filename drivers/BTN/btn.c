/*
Header to define button module logic
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "BTN.h"

/* ----------------------------------------------------------------------------
                                    Constants
---------------------------------------------------------------------------- */
#define BTN_DEBOUNCE_MS   20

/* ----------------------------------------------------------------------------
                                  Macro Helpers
---------------------------------------------------------------------------- */
#define BTN0_NODE             DT_ALIAS(sw0)
#define BTN1_NODE             DT_ALIAS(sw1)
#define BTN2_NODE             DT_ALIAS(sw2)
#define BTN3_NODE             DT_ALIAS(sw3)

#define IS_INVALID_BTN(btn)   (btn >= NUM_BTNS || btn < 0)

/* ----------------------------------------------------------------------------
                                    Types
---------------------------------------------------------------------------- */
typedef struct btn_gpio_t {
  struct gpio_dt_spec spec; 
  volatile bool pressed;
  struct gpio_callback cb;
  struct k_work_delayable work;
} btn_gpio;

/* ----------------------------------------------------------------------------
                            Private Function Prototypes
---------------------------------------------------------------------------- */
static int _btn_config(btn_gpio *btn);

static void _btn_interrupt_service_routine(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

static void _btn_debounce(struct k_work *work);

/* ----------------------------------------------------------------------------
                                Global States
---------------------------------------------------------------------------- */
static btn_gpio _btn0 = {.spec=GPIO_DT_SPEC_GET(BTN0_NODE, gpios), .pressed=false};
static btn_gpio _btn1 = {.spec=GPIO_DT_SPEC_GET(BTN1_NODE, gpios), .pressed=false};
static btn_gpio _btn2 = {.spec=GPIO_DT_SPEC_GET(BTN2_NODE, gpios), .pressed=false};
static btn_gpio _btn3 = {.spec=GPIO_DT_SPEC_GET(BTN3_NODE, gpios), .pressed=false};
static btn_gpio *_btns[NUM_BTNS] = {&_btn0, &_btn1, &_btn2, &_btn3};

/* ----------------------------------------------------------------------------
                              Private Functions
---------------------------------------------------------------------------- */
/**
 * @brief Configures a gpio spec as a button
 * 
 * @param [in] btn the btn_gpio object to configure
 * 
 * @return Error code, < 0 on failures
 */
static int _btn_config(btn_gpio *btn) {
  if (!gpio_is_ready_dt(&btn->spec)) {
		return -EIO;
	} else if (0 > gpio_pin_configure_dt(&btn->spec, GPIO_INPUT)) {
		return -EIO;
  } else if (0 > gpio_pin_interrupt_configure_dt(&btn->spec, GPIO_INT_EDGE_TO_ACTIVE)) {
		return -EIO;
  } else {
    gpio_init_callback(&btn->cb, _btn_interrupt_service_routine, BIT(btn->spec.pin));
    gpio_add_callback(btn->spec.port, &btn->cb);
    k_work_init_delayable(&btn->work, _btn_debounce);
    return 0;
  }
}

/**
 * @brief Invoked as an interrupt when a button goes to the active state (high)
 * 
 * @param [in] dev The GPIO port that triggered the interrupt
 * @param [in] cb A pointer to the registered callback structure for this ISR
 * @param [in] pins A bitmask for all the GPIO pins that triggered this interrupt
 */
static void _btn_interrupt_service_routine(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    if (pins & BIT(_btns[i]->spec.pin)) {
      k_work_reschedule(&_btns[i]->work, K_MSEC(BTN_DEBOUNCE_MS));
    }
  }
  return;
}

/**
 * @brief Called once the button has been debounced, sets button pressed state
 * 
 * @param [in] work A k_work struct contained by a k_work_delayable inside a btn_gpio struct
 */
static void _btn_debounce(struct k_work *_work) {
  struct k_work_delayable *dwork = CONTAINER_OF(_work, struct k_work_delayable, work);
  btn_gpio *btn = CONTAINER_OF(dwork, btn_gpio, work);

  if (gpio_pin_get_dt(&btn->spec)) {
    btn->pressed = true;
  }
}

/* ----------------------------------------------------------------------------
                              Public Functions
---------------------------------------------------------------------------- */
/**
 * @brief Inits all buttons
 * 
 * @return Error code, < 0 on failures
 */
int BTN_init() {
  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    int rv = _btn_config(_btns[i]);
    if (rv < 0) {
      return rv;
    }
  }
  return 0;
}

/**
 * @brief Checks if the given button is currently being pressed
 * 
 * @param [in] btn Which button to check
 * 
 * @return true if btn is being pressed
 */
bool BTN_is_pressed(btn_id btn) {
  if (IS_INVALID_BTN(btn)) {
    return false;
  } else if (0 < gpio_pin_get_dt(&_btns[btn]->spec)) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Checks if the given button has been pressed, clears internal state flag.
 *        Equivalent to calling BTN_check_pressed(BTNx) then calling BTN_clear_pressed(BTNx)
 * 
 * @param [in] btn Which button to check
 * 
 * @return true if btn has been pressed
 */
bool BTN_check_clear_pressed(btn_id btn) {
  if (IS_INVALID_BTN(btn)) {
    return false;
  } else {
    bool was_pressed = _btns[btn]->pressed;
    _btns[btn]->pressed = false;
    return was_pressed;
  }
}

/**
 * @brief Checks if the given button has been pressed, doesn't clear the internal state flag
 * 
 * @param [in] btn Which button to check
 * 
 * @return true if btn has been pressed
 */
bool BTN_check_pressed(btn_id btn) {
  if (IS_INVALID_BTN(btn)) {
    return false;
  } else {
    return _btns[btn]->pressed;
  }
}

/**
 * @brief Clears the internal state flag of a given button
 * 
 * @param [in] btn Which button to clear
 */
void BTN_clear_pressed(btn_id btn) {
  if (IS_INVALID_BTN(btn)) {
    return;
  } else {
    _btns[btn]->pressed = false;
    return;
  }
}
