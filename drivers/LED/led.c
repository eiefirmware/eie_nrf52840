/*
Header to define led module logic
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "LED.h"

/* ----------------------------------------------------------------------------
                                    Constants
---------------------------------------------------------------------------- */
#define LED_BLINK_STACK_SIZE      384
#define LED_BLINK_PRIORITY        1
#define LED_COUNTER_UNIT          100 // Units per ms (1 unit == 10us)
#define LED_COUNTER_HALF_PERIOD   500 * LED_COUNTER_UNIT // Units per half second (1 second / 2 == 500ms)

#define LED0_NODE   DT_ALIAS(led0)
#define LED1_NODE   DT_ALIAS(led1)
#define LED2_NODE   DT_ALIAS(led2)
#define LED3_NODE   DT_ALIAS(led3)

/* ----------------------------------------------------------------------------
                                    Types
---------------------------------------------------------------------------- */
typedef struct led_blink_t {
  uint16_t half_period; // Units of 10us
  uint16_t offset; // Units of 10us
} led_blink;

typedef struct led_gpio_t {
  struct gpio_dt_spec spec; 
  led_blink blink;
} led_gpio;

typedef struct blink_thread_t {
  struct k_thread thread;
  k_tid_t id;
  uint8_t led_bitmask;
} blink_thread;

/* ----------------------------------------------------------------------------
                            Private Function Prototypes
---------------------------------------------------------------------------- */
static int _led_config(const led_gpio *led);

void _led_blink_loop(void *led, void *p2, void *p3);

/* ----------------------------------------------------------------------------
                                Global States
---------------------------------------------------------------------------- */
static led_gpio _led0 = {.spec=GPIO_DT_SPEC_GET(LED0_NODE, gpios)};
static led_gpio _led1 = {.spec=GPIO_DT_SPEC_GET(LED1_NODE, gpios)};
static led_gpio _led2 = {.spec=GPIO_DT_SPEC_GET(LED2_NODE, gpios)};
static led_gpio _led3 = {.spec=GPIO_DT_SPEC_GET(LED3_NODE, gpios)};
static led_gpio *_leds[NUM_LEDS] = {&_led0, &_led1, &_led2, &_led3};

static blink_thread _led_blink_thread = {.led_bitmask=0};
K_THREAD_STACK_DEFINE(_led_blink_stack, LED_BLINK_STACK_SIZE);

/* ----------------------------------------------------------------------------
                              Private Functions
---------------------------------------------------------------------------- */
/**
 * @brief Configures a gpio spec as an led
 * 
 * @param [in] led the led_gpio object to configure
 * 
 * @return Error code, < 0 on failures
 */
static int _led_config(const led_gpio *led) {
  if (!gpio_is_ready_dt(&led->spec)) {
		return -EIO;
	} else if (0 > gpio_pin_configure_dt(&led->spec, GPIO_OUTPUT_INACTIVE)) {
		return -EIO;
  } else {
    return 0;
  }
}

/**
 * @brief Handles blinking all LEDs
 * 
 * @param [in] p1 Unused thread parameter 1
 * @param [in] p2 Unused thread parameter 2
 * @param [in] p2 Unused thread parameter 3
 */
void _led_blink_loop(void *p1 __attribute__((unused)), void *p2 __attribute__((unused)), void *p3 __attribute__((unused))) {
  uint16_t min_half_period = LED_COUNTER_HALF_PERIOD / LED_16HZ;

  while (1) {
    k_msleep(min_half_period / LED_COUNTER_UNIT);

    for (int i = 0; i < NUM_LEDS; i++) {
      if (_led_blink_thread.led_bitmask & BIT(i)) {
        _leds[i]->blink.offset += min_half_period;
        if (_leds[i]->blink.offset >= _leds[i]->blink.half_period){
          _leds[i]->blink.offset = 0;
          LED_toggle(i);
        }
      }
    }
  }
}

/* ----------------------------------------------------------------------------
                              Public Functions
---------------------------------------------------------------------------- */
/**
 * @brief Inits all LEDs
 * 
 * @return Error code, < 0 on failures
 */
int LED_init() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int rv = _led_config(_leds[i]);
    if (rv < 0) {
      return rv;
    }
  }

  _led_blink_thread.id = k_thread_create(
    &_led_blink_thread.thread,
    _led_blink_stack,
    K_THREAD_STACK_SIZEOF(_led_blink_stack),
    _led_blink_loop,
    NULL, NULL, NULL,
    LED_BLINK_PRIORITY,
    0,
    K_NO_WAIT
  );
  k_thread_suspend(_led_blink_thread.id);
  
  return 0;
}

/**
 * @brief Toggle specified led
 * 
 * @param [in] led The led instance to toggle
 * 
 * @return Error code, < 0 on failures
 */
int LED_toggle(led_id led) {
  if (led >= NUM_LEDS || led < 0) {
    return -EINVAL;
  } else {
    return gpio_pin_toggle_dt(&_leds[led]->spec);
  }
}

/**
 * @brief Set specified led to given state
 * 
 * @param [in] led The led instance to set
 * @param [in] new_state The state to set the led to
 * 
 * @return Error code, < 0 on failures
 */
int LED_set(led_id led, led_state new_state) {
  if (led >= NUM_LEDS || led < 0) {
    return -EINVAL;
  }

  _led_blink_thread.led_bitmask &= ~BIT(led);
  if (!_led_blink_thread.led_bitmask) {
    k_thread_suspend(_led_blink_thread.id);
  }

  return gpio_pin_set_dt(&_leds[led]->spec, new_state);
}

/**
 * @brief Blinks the given LED at the given frequency
 * 
 * @param [in] led The led instance to blink
 * @param [in] frequency The frequency to blink the led at
 */
void LED_blink(led_id led, led_frequency frequency) {
  if (led >= NUM_LEDS || led < 0) {
    return;
  } else if (frequency > LED_16HZ || frequency <= 0) {
    return;
  }

  _leds[led]->blink.half_period = LED_COUNTER_HALF_PERIOD / frequency;
  _leds[led]->blink.offset = 0;

  if (!_led_blink_thread.led_bitmask) {
    k_thread_resume(_led_blink_thread.id);
  }

  _led_blink_thread.led_bitmask |= BIT(led);
}
