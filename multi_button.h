/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#ifndef _MULTI_BUTTON_H_
#define _MULTI_BUTTON_H_

#include <stdint.h>
#include <string.h>

// Configuration constants - can be modified according to your needs
#define TICKS_INTERVAL          5    // ms - timer interrupt interval
#define DEBOUNCE_TICKS          3    // MAX 7 (0 ~ 7) - debounce filter depth
#define SHORT_TICKS             (300 / TICKS_INTERVAL)   // short press threshold
#define LONG_TICKS              (1000 / TICKS_INTERVAL)  // long press threshold
#define PRESS_REPEAT_MAX_NUM    15   // maximum repeat counter value

// Forward declaration
typedef struct _Button Button;

// Button callback function type
typedef void (*BtnCallback)(Button* btn_handle);

// Button event types
typedef enum {
	BTN_PRESS_DOWN = 0,     // button pressed down
	BTN_PRESS_UP,           // button released
	BTN_PRESS_REPEAT,       // repeated press detected
	BTN_SINGLE_CLICK,       // single click completed
	BTN_DOUBLE_CLICK,       // double click completed
	BTN_LONG_PRESS_START,   // long press started
	BTN_LONG_PRESS_HOLD,    // long press holding
	BTN_EVENT_COUNT,        // total number of events
	BTN_NONE_PRESS          // no event
} ButtonEvent;

// Button state machine states
typedef enum {
	BTN_STATE_IDLE = 0,     // idle state
	BTN_STATE_PRESS,        // pressed state
	BTN_STATE_RELEASE,      // released state waiting for timeout
	BTN_STATE_REPEAT,       // repeat press state
	BTN_STATE_LONG_HOLD     // long press hold state
} ButtonState;

// Button structure
struct _Button {
	uint16_t ticks;                     // tick counter
	uint8_t  repeat : 4;                // repeat counter (0-15)
	uint8_t  event : 4;                 // current event (0-15)
	uint8_t  state : 3;                 // state machine state (0-7)
	uint8_t  debounce_cnt : 3;          // debounce counter (0-7)
	uint8_t  active_level : 1;          // active GPIO level (0 or 1)
	uint8_t  button_level : 1;          // current button level
	uint8_t  button_id;                 // button identifier
	uint8_t  (*hal_button_level)(uint8_t button_id);  // HAL function to read GPIO
	BtnCallback cb[BTN_EVENT_COUNT];    // callback function array
	Button* next;                       // next button in linked list
};

#ifdef __cplusplus
extern "C" {
#endif

// Public API functions
void button_init(Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id);
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb);
void button_detach(Button* handle, ButtonEvent event);
ButtonEvent button_get_event(Button* handle);
int  button_start(Button* handle);
void button_stop(Button* handle);
void button_ticks(void);

// Utility functions
uint8_t button_get_repeat_count(Button* handle);
void button_reset(Button* handle);
int button_is_pressed(Button* handle);

#ifdef __cplusplus
}
#endif

#endif
