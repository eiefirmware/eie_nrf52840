/*
Header to define led module logic
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <inttypes.h>

#include "LED.h"

/* ----------------------------------------------------------------------------
                                    Constants
---------------------------------------------------------------------------- */
#define LED_BLINK_STACK_SIZE      384
#define LED_BLINK_PRIORITY        1
#define LED_COUNTER_UNIT          100 // Units per ms (1 unit == 10us)
#define LED_COUNTER_HALF_PERIOD   500 * LED_COUNTER_UNIT // Units per half second (1 second / 2 == 500ms)

#define PWM_MAX_DUTY_CYCLE        100 // Valid duty cycle range for this application is 0 - 100

/* ----------------------------------------------------------------------------
                                  Macro Helpers
---------------------------------------------------------------------------- */
#define LED0_NODE             DT_ALIAS(pwm_led0)
#define LED1_NODE             DT_ALIAS(pwm_led1)
#define LED2_NODE             DT_ALIAS(pwm_led2)
#define LED3_NODE             DT_ALIAS(pwm_led3)

#define IS_INVALID_LED(led)   (led >= NUM_LEDS || led < 0)

/* ----------------------------------------------------------------------------
                                    Types
---------------------------------------------------------------------------- */
typedef struct led_blink_t {
  uint16_t half_period; // Units of 10us
  uint16_t offset; // Units of 10us
} led_blink;

typedef struct led_t {
  struct pwm_dt_spec spec; 
  led_blink blink;
  uint8_t current_duty_cycle; // Valid from 0 - 100
} led_type;

typedef struct blink_thread_t {
  struct k_thread thread;
  k_tid_t id;
  uint8_t led_bitmask;
} blink_thread;

/* ----------------------------------------------------------------------------
                            Private Function Prototypes
---------------------------------------------------------------------------- */
static int _led_pwm_preserve_blink(led_id led, uint8_t duty_cycle);

static void _led_halt_blink(led_id led);

static void _led_blink_loop(void *led, void *p2, void *p3);

/* ----------------------------------------------------------------------------
                                Global States
---------------------------------------------------------------------------- */
static led_type _led0 = {.spec=PWM_DT_SPEC_GET(LED0_NODE), .current_duty_cycle=0};
static led_type _led1 = {.spec=PWM_DT_SPEC_GET(LED1_NODE), .current_duty_cycle=0};
static led_type _led2 = {.spec=PWM_DT_SPEC_GET(LED2_NODE), .current_duty_cycle=0};
static led_type _led3 = {.spec=PWM_DT_SPEC_GET(LED3_NODE), .current_duty_cycle=0};
static led_type *_leds[NUM_LEDS] = {&_led0, &_led1, &_led2, &_led3};

static blink_thread _led_blink_thread = {.led_bitmask=0};
K_THREAD_STACK_DEFINE(_led_blink_stack, LED_BLINK_STACK_SIZE);

/* ----------------------------------------------------------------------------
                              Private Functions
---------------------------------------------------------------------------- */
/**
 * @brief Sets the LED to the given duty cycle, doesn't halt blinking
 * 
 * @param [in] led the LED to set the duty cycle of
 * @param [in] duty_cycle the duty cycle to set the LED to
 * 
 * @return Error code, < 0 on failures
 */
static int _led_pwm_preserve_blink(led_id led, uint8_t duty_cycle) {
  if (IS_INVALID_LED(led)) {
    return -EINVAL;
  }
  uint8_t clamped_duty_cycle = PWM_MAX_DUTY_CYCLE < duty_cycle ? PWM_MAX_DUTY_CYCLE : duty_cycle;
  uint32_t pwm_step = _leds[led]->spec.period / PWM_MAX_DUTY_CYCLE;
  // Subtract clamped duty cycle as leds are active low
  return pwm_set_pulse_dt(&_leds[led]->spec, pwm_step * (PWM_MAX_DUTY_CYCLE - clamped_duty_cycle));
}

/**
 * @brief Halts blinking for the given LED
 * 
 * @param [in] led the LED instance to halt blinking for
 */
static void _led_halt_blink(led_id led) {
  if (IS_INVALID_LED(led)) {
    return;
  }

  _led_blink_thread.led_bitmask &= ~BIT(led);
  if (!_led_blink_thread.led_bitmask) {
    k_thread_suspend(_led_blink_thread.id);
  }
}

/**
 * @brief Handles blinking all LEDs
 * 
 * @param [in] p1 Unused thread parameter 1
 * @param [in] p2 Unused thread parameter 2
 * @param [in] p2 Unused thread parameter 3
 */
static void _led_blink_loop(void *p1 __attribute__((unused)), void *p2 __attribute__((unused)), void *p3 __attribute__((unused))) {
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
    int rv = pwm_is_ready_dt(&_leds[i]->spec);
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
 * @brief Toggle specified LED
 * 
 * @param [in] led The LED instance to toggle
 * 
 * @return Error code, < 0 on failures
 */
int LED_toggle(led_id led) {
  if (IS_INVALID_LED(led)) {
    return -EINVAL;
  } else {
    if (0 == _leds[led]->current_duty_cycle) {
      _leds[led]->current_duty_cycle = PWM_MAX_DUTY_CYCLE;
    } else {
      _leds[led]->current_duty_cycle = 0;
    }
    return _led_pwm_preserve_blink(led, _leds[led]->current_duty_cycle);
  }
}

/**
 * @brief Set specified LED to given state
 * 
 * @param [in] led The LED instance to set
 * @param [in] new_state The state to set the led to
 * 
 * @return Error code, < 0 on failures
 */
int LED_set(led_id led, led_state new_state) {
  if (IS_INVALID_LED(led)) {
    return -EINVAL;
  }

  _led_halt_blink(led);

  _leds[led]->current_duty_cycle = (0 == new_state) ? 0 : PWM_MAX_DUTY_CYCLE;
  return LED_pwm(led, _leds[led]->current_duty_cycle);
}

/**
 * @brief Set specified LED to given pwm duty cycle
 * 
 * @param [in] led The LED instance to set the pwm duty cycle of
 * @param [in] duty_cycle The duty cycle to set the LED to, expects 0 - 100 only
 * 
 * @return Error code, < 0 on failures
 */
int LED_pwm(led_id led, uint8_t duty_cycle) {
  if (IS_INVALID_LED(led)) {
    return -EINVAL;
  }

  _led_halt_blink(led);

  return _led_pwm_preserve_blink(led, duty_cycle);
}

/**
 * @brief Blinks the given LED at the given frequency
 * 
 * @param [in] led The LED instance to blink
 * @param [in] frequency The frequency to blink the led at
 */
void LED_blink(led_id led, led_frequency frequency) {
  if (IS_INVALID_LED(led)) {
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
