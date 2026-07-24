/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#include "multi_button.h"

// Macro for callback execution with null check, passes user_data
#define EVENT_CB(ev)   do { if (handle->cb[ev]) handle->cb[ev](handle, handle->user_data); } while(0)

// Button handle list head
static Button* head_handle = NULL;

// Forward declarations
static uint32_t button_handler(Button* handle, uint32_t elapsed_ms);
static inline uint8_t button_read_level(Button* handle);
static uint32_t ticks_to_ms(uint32_t ticks);
static uint32_t ticks_remaining_ms(uint16_t elapsed_ticks,
				   uint32_t remainder_ms,
				   uint32_t target_ticks);

/**
  * @brief  Initialize the button struct handle
  * @param  handle: the button handle struct
  * @param  pin_level: read the HAL GPIO of the connected button level
  * @param  active_level: pressed GPIO level
  * @param  button_id: the button id
  * @retval None
  */
void button_init(Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id)
{
	if (!handle || !pin_level) return;  // parameter validation

	memset(handle, 0, sizeof(Button));
	handle->event = (uint8_t)BTN_NONE_PRESS;
	handle->hal_button_level = pin_level;
	handle->button_level = !active_level;  // initialize to opposite of active level
	handle->active_level = active_level;
	handle->button_id = button_id;
	handle->state = BTN_STATE_IDLE;
	// user_data is zeroed by memset
}

/**
  * @brief  Attach the button event callback function
  * @param  handle: the button handle struct
  * @param  event: trigger event type
  * @param  cb: callback function
  * @param  user_data: user context pointer passed to callback (stored per-button)
  * @retval None
  */
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb, void* user_data)
{
	if (!handle || event >= BTN_EVENT_COUNT) return;  // parameter validation
	handle->cb[event] = cb;
	handle->user_data = user_data;
}

/**
  * @brief  Detach the button event callback function
  * @param  handle: the button handle struct
  * @param  event: trigger event type
  * @retval None
  */
void button_detach(Button* handle, ButtonEvent event)
{
	if (!handle || event >= BTN_EVENT_COUNT) return;  // parameter validation
	handle->cb[event] = NULL;
}

/**
  * @brief  Get the button event that happened
  * @param  handle: the button handle struct
  * @retval button event
  */
ButtonEvent button_get_event(Button* handle)
{
	if (!handle) return BTN_NONE_PRESS;
	return (ButtonEvent)(handle->event);
}

/**
  * @brief  Get the repeat count of button presses
  * @param  handle: the button handle struct
  * @retval repeat count
  */
uint8_t button_get_repeat_count(Button* handle)
{
	if (!handle) return 0;
	return handle->repeat;
}

/**
  * @brief  Reset button state to idle
  * @param  handle: the button handle struct
  * @retval None
  */
void button_reset(Button* handle)
{
	if (!handle) return;
	handle->state = BTN_STATE_IDLE;
	handle->ticks = 0;
	handle->tick_remainder_ms = 0;
	handle->repeat = 0;
	handle->event = (uint8_t)BTN_NONE_PRESS;
	handle->debounce_cnt = 0;
	handle->debounce_elapsed_ms = 0;
}

/**
  * @brief  Check if button is currently pressed
  * @param  handle: the button handle struct
  * @retval 1: pressed, 0: not pressed, -1: error
  */
int button_is_pressed(Button* handle)
{
	if (!handle) return -1;
	return (handle->button_level == handle->active_level) ? 1 : 0;
}

/**
  * @brief  Read button level with inline optimization
  * @param  handle: the button handle struct
  * @retval button level
  */
static inline uint8_t button_read_level(Button* handle)
{
	return handle->hal_button_level(handle->button_id);
}

/**
  * @brief  Button driver core function, driver state machine
  * @param  handle: the button handle struct
  * @retval None
  */
static uint32_t ticks_to_ms(uint32_t ticks)
{
	if (ticks > UINT32_MAX / TICKS_INTERVAL) {
		return UINT32_MAX;
	}
	return ticks * TICKS_INTERVAL;
}

static uint32_t ticks_remaining_ms(uint16_t elapsed_ticks,
				   uint32_t remainder_ms,
				   uint32_t target_ticks)
{
	uint32_t remaining_ms;

	if (elapsed_ticks >= target_ticks) {
		return 1U;
	}

	remaining_ms = ticks_to_ms(target_ticks - elapsed_ticks);
	if (remainder_ms >= remaining_ms) {
		return 1U;
	}
	return remaining_ms - remainder_ms;
}

static uint32_t button_handler(Button* handle, uint32_t elapsed_ms)
{
	uint32_t next_time = 0;
	uint32_t debounce_time_ms = ticks_to_ms(DEBOUNCE_TICKS);
	uint8_t read_gpio_level = button_read_level(handle);

	// Accumulate actual elapsed time without losing sub-tick remainders.
	if (handle->state > BTN_STATE_IDLE) {
		uint32_t total_ms = (uint32_t)handle->tick_remainder_ms + elapsed_ms;
		uint32_t elapsed_ticks = total_ms / TICKS_INTERVAL;
		handle->tick_remainder_ms = total_ms % TICKS_INTERVAL;
		if (elapsed_ticks > (uint32_t)UINT16_MAX - handle->ticks) {
			handle->ticks = UINT16_MAX;
		} else {
			handle->ticks = (uint16_t)(handle->ticks + elapsed_ticks);
		}
	}

	/*
	 * Deferred debounce: after the first changed sample, wait for the complete
	 * debounce interval and read once more. Low-power users therefore need only
	 * one timer wake-up instead of waking for every debounce sample.
	 */
	if (read_gpio_level != handle->button_level) {
		if (handle->debounce_cnt == 0) {
			handle->debounce_cnt = 1;
			handle->debounce_elapsed_ms = 0;
			return debounce_time_ms;
		}

		if (elapsed_ms >= debounce_time_ms - handle->debounce_elapsed_ms) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
			handle->debounce_elapsed_ms = 0;
		} else {
			handle->debounce_elapsed_ms += elapsed_ms;
			return debounce_time_ms - handle->debounce_elapsed_ms;
		}
	} else {
		// The signal returned to its stable level before the deadline.
		handle->debounce_cnt = 0;
		handle->debounce_elapsed_ms = 0;
	}

	/* State machine */
	switch (handle->state) {
	case BTN_STATE_IDLE:
		if (handle->button_level == handle->active_level) {
			// Button press detected
			handle->event = (uint8_t)BTN_PRESS_DOWN;
			EVENT_CB(BTN_PRESS_DOWN);
			handle->ticks = 0;
			handle->tick_remainder_ms = 0;
			handle->repeat = 1;
			handle->state = BTN_STATE_PRESS;
			next_time = ticks_to_ms(LONG_TICKS);
		} else {
			handle->event = (uint8_t)BTN_NONE_PRESS;
		}
		break;

	case BTN_STATE_PRESS:
		if (handle->button_level != handle->active_level) {
			// Button released
			handle->event = (uint8_t)BTN_PRESS_UP;
			EVENT_CB(BTN_PRESS_UP);
			handle->ticks = 0;
			handle->tick_remainder_ms = 0;
#if MULTIBUTTON_ENABLE_DOUBLE_CLICK
			handle->state = BTN_STATE_RELEASE;
			next_time = ticks_to_ms(SHORT_TICKS);
#else
			handle->repeat = 0;
			handle->state = BTN_STATE_IDLE;
			handle->event = (uint8_t)BTN_SINGLE_CLICK;
			EVENT_CB(BTN_SINGLE_CLICK);
#endif
		} else if (handle->ticks >= LONG_TICKS) {
			// Long press detected
			handle->event = (uint8_t)BTN_LONG_PRESS_START;
			EVENT_CB(BTN_LONG_PRESS_START);
			handle->state = BTN_STATE_LONG_HOLD;
			if (handle->cb[BTN_LONG_PRESS_HOLD]) {
				next_time = ticks_to_ms(LONG_HOLD_TICKS);
			}
		} else {
			next_time = ticks_remaining_ms(handle->ticks,
						       handle->tick_remainder_ms,
						       LONG_TICKS);
		}
		break;

#if MULTIBUTTON_ENABLE_DOUBLE_CLICK
	case BTN_STATE_RELEASE:
		if (handle->button_level == handle->active_level) {
			// Button pressed again
			handle->event = (uint8_t)BTN_PRESS_DOWN;
			EVENT_CB(BTN_PRESS_DOWN);
			if (handle->repeat < PRESS_REPEAT_MAX_NUM) {
				handle->repeat++;
			}
			handle->event = (uint8_t)BTN_PRESS_REPEAT;
			EVENT_CB(BTN_PRESS_REPEAT);
			handle->ticks = 0;
			handle->tick_remainder_ms = 0;
			handle->state = BTN_STATE_REPEAT;
			next_time = ticks_to_ms(SHORT_TICKS);
		} else if (handle->ticks >= SHORT_TICKS) {
			// Timeout reached, determine click type
			if (handle->repeat == 1) {
				handle->event = (uint8_t)BTN_SINGLE_CLICK;
				EVENT_CB(BTN_SINGLE_CLICK);
			} else if (handle->repeat == 2) {
				handle->event = (uint8_t)BTN_DOUBLE_CLICK;
				EVENT_CB(BTN_DOUBLE_CLICK);
			}
			handle->state = BTN_STATE_IDLE;
			handle->ticks = 0;
			handle->tick_remainder_ms = 0;
		} else {
			next_time = ticks_remaining_ms(handle->ticks,
						       handle->tick_remainder_ms,
						       SHORT_TICKS);
		}
		break;

	case BTN_STATE_REPEAT:
		if (handle->button_level != handle->active_level) {
			// Button released
			handle->event = (uint8_t)BTN_PRESS_UP;
			EVENT_CB(BTN_PRESS_UP);
			if (handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->tick_remainder_ms = 0;
				handle->state = BTN_STATE_RELEASE;  // Continue waiting for more presses
				next_time = ticks_to_ms(SHORT_TICKS);
			} else {
				handle->state = BTN_STATE_IDLE;  // End of sequence
			}
		} else if (handle->ticks >= SHORT_TICKS) {
			// Continue timing the second press toward the long-press threshold.
			handle->state = BTN_STATE_PRESS;
			next_time = ticks_remaining_ms(handle->ticks,
						       handle->tick_remainder_ms,
						       LONG_TICKS);
		} else {
			next_time = ticks_remaining_ms(handle->ticks,
						       handle->tick_remainder_ms,
						       SHORT_TICKS);
		}
		break;
#endif

	case BTN_STATE_LONG_HOLD:
		if (handle->button_level == handle->active_level) {
			// Continue holding
			handle->event = (uint8_t)BTN_LONG_PRESS_HOLD;
			EVENT_CB(BTN_LONG_PRESS_HOLD);
			if (handle->cb[BTN_LONG_PRESS_HOLD]) {
				next_time = ticks_to_ms(LONG_HOLD_TICKS);
			}
		} else {
			// Released from long press
			handle->event = (uint8_t)BTN_PRESS_UP;
			EVENT_CB(BTN_PRESS_UP);
			handle->state = BTN_STATE_IDLE;
			handle->ticks = 0;
			handle->tick_remainder_ms = 0;
		}
		break;

	default:
		// Invalid state, reset to idle
		handle->state = BTN_STATE_IDLE;
		break;
	}
	return next_time;
}

/**
  * @brief  Start the button work, add the handle into work list
  * @param  handle: target handle struct
  * @retval 0: succeed, -1: already exist, -2: invalid parameter
  */
int button_start(Button* handle)
{
	if (!handle) return -2;  // invalid parameter

	MULTIBUTTON_LOCK();
	Button* target = head_handle;
	while (target) {
		if (target == handle) {
			MULTIBUTTON_UNLOCK();
			return -1;  // already exist
		}
		target = target->next;
	}

	handle->next = head_handle;
	head_handle = handle;
	MULTIBUTTON_UNLOCK();
	return 0;
}

/**
  * @brief  Stop the button work, remove the handle from work list
  * @param  handle: target handle struct
  * @retval None
  */
void button_stop(Button* handle)
{
	if (!handle) return;  // parameter validation

	MULTIBUTTON_LOCK();
	Button** curr;
	for (curr = &head_handle; *curr; ) {
		Button* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
			entry->next = NULL;  // clear next pointer
			MULTIBUTTON_UNLOCK();
			return;
		} else {
			curr = &entry->next;
		}
	}
	MULTIBUTTON_UNLOCK();
}

/**
  * @brief  Background ticks, timer repeat invoking interval 5ms
  *         Callbacks are executed outside the lock so they may safely
  *         call button_start()/button_stop() without deadlock risk.
  * @param  None
  * @retval None
  */
void button_ticks(void)
{
	(void)button_ticks_low_power(TICKS_INTERVAL);
}

uint32_t button_ticks_low_power(uint32_t elapsed_ms)
{
	Button* target;
	Button* next;
	uint32_t button_next;
	uint32_t next_scan_time = UINT32_MAX;

	MULTIBUTTON_LOCK();
	target = head_handle;
	MULTIBUTTON_UNLOCK();

	while (target) {
		MULTIBUTTON_LOCK();
		next = target->next;
		MULTIBUTTON_UNLOCK();

		button_next = button_handler(target, elapsed_ms);
		if (button_next != 0 && button_next < next_scan_time) {
			next_scan_time = button_next;
		}
		target = next;
	}

	return next_scan_time == UINT32_MAX ? 0 : next_scan_time;
}
