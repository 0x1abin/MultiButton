/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#include "multi_button.h"

// Macro for callback execution with null check
#define EVENT_CB(ev)   do { if(handle->cb[ev]) handle->cb[ev](handle); } while(0)

// Button handle list head
static Button* head_handle = NULL;

// Forward declarations
static void button_handler(Button* handle);
static inline uint8_t button_read_level(Button* handle);

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
}

/**
  * @brief  Attach the button event callback function
  * @param  handle: the button handle struct
  * @param  event: trigger event type
  * @param  cb: callback function
  * @retval None
  */
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb)
{
	if (!handle || event >= BTN_EVENT_COUNT) return;  // parameter validation
	handle->cb[event] = cb;
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
	handle->repeat = 0;
	handle->event = (uint8_t)BTN_NONE_PRESS;
	handle->debounce_cnt = 0;
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
static void button_handler(Button* handle)
{
	uint8_t read_gpio_level = button_read_level(handle);

	// Increment ticks counter when not in idle state
	if (handle->state > BTN_STATE_IDLE) {
		handle->ticks++;
	}

	/*------------Button debounce handling---------------*/
	if (read_gpio_level != handle->button_level) {
		// Continue reading same new level for debounce
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} else {
		// Level not changed, reset counter
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case BTN_STATE_IDLE:
		if (handle->button_level == handle->active_level) {
			// Button press detected
			handle->event = (uint8_t)BTN_PRESS_DOWN;
			EVENT_CB(BTN_PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = BTN_STATE_PRESS;
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
			handle->state = BTN_STATE_RELEASE;
		} else if (handle->ticks > LONG_TICKS) {
			// Long press detected
			handle->event = (uint8_t)BTN_LONG_PRESS_START;
			EVENT_CB(BTN_LONG_PRESS_START);
			handle->state = BTN_STATE_LONG_HOLD;
		}
		break;

	case BTN_STATE_RELEASE:
		if (handle->button_level == handle->active_level) {
			// Button pressed again
			handle->event = (uint8_t)BTN_PRESS_DOWN;
			EVENT_CB(BTN_PRESS_DOWN);
			if (handle->repeat < PRESS_REPEAT_MAX_NUM) {
				handle->repeat++;
			}
			EVENT_CB(BTN_PRESS_REPEAT);
			handle->ticks = 0;
			handle->state = BTN_STATE_REPEAT;
		} else if (handle->ticks > SHORT_TICKS) {
			// Timeout reached, determine click type
			if (handle->repeat == 1) {
				handle->event = (uint8_t)BTN_SINGLE_CLICK;
				EVENT_CB(BTN_SINGLE_CLICK);
			} else if (handle->repeat == 2) {
				handle->event = (uint8_t)BTN_DOUBLE_CLICK;
				EVENT_CB(BTN_DOUBLE_CLICK);
			}
			handle->state = BTN_STATE_IDLE;
		}
		break;

	case BTN_STATE_REPEAT:
		if (handle->button_level != handle->active_level) {
			// Button released
			handle->event = (uint8_t)BTN_PRESS_UP;
			EVENT_CB(BTN_PRESS_UP);
			if (handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = BTN_STATE_RELEASE;  // Continue waiting for more presses
			} else {
				handle->state = BTN_STATE_IDLE;  // End of sequence
			}
		} else if (handle->ticks > SHORT_TICKS) {
			// Held down too long, treat as normal press
			handle->state = BTN_STATE_PRESS;
		}
		break;

	case BTN_STATE_LONG_HOLD:
		if (handle->button_level == handle->active_level) {
			// Continue holding
			handle->event = (uint8_t)BTN_LONG_PRESS_HOLD;
			EVENT_CB(BTN_LONG_PRESS_HOLD);
		} else {
			// Released from long press
			handle->event = (uint8_t)BTN_PRESS_UP;
			EVENT_CB(BTN_PRESS_UP);
			handle->state = BTN_STATE_IDLE;
		}
		break;

	default:
		// Invalid state, reset to idle
		handle->state = BTN_STATE_IDLE;
		break;
	}
}

/**
  * @brief  Start the button work, add the handle into work list
  * @param  handle: target handle struct
  * @retval 0: succeed, -1: already exist, -2: invalid parameter
  */
int button_start(Button* handle)
{
	if (!handle) return -2;  // invalid parameter
	
	Button* target = head_handle;
	while (target) {
		if (target == handle) return -1;  // already exist
		target = target->next;
	}
	
	handle->next = head_handle;
	head_handle = handle;
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
	
	Button** curr;
	for (curr = &head_handle; *curr; ) {
		Button* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
			entry->next = NULL;  // clear next pointer
			return;
		} else {
			curr = &entry->next;
		}
	}
}

/**
  * @brief  Background ticks, timer repeat invoking interval 5ms
  * @param  None
  * @retval None
  */
void button_ticks(void)
{
	Button* target;
	for (target = head_handle; target; target = target->next) {
		button_handler(target);
	}
}
