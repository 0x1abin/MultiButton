/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#ifndef MULTI_BUTTON_H
#define MULTI_BUTTON_H

#include <stdint.h>
#include <string.h>

// Version information
#define MULTIBUTTON_VERSION_MAJOR 1
#define MULTIBUTTON_VERSION_MINOR 1
#define MULTIBUTTON_VERSION_PATCH 1

// Configuration constants - can be modified according to your needs
#define TICKS_INTERVAL          5    // ms - timer interrupt interval
#define DEBOUNCE_TICKS          3    // MAX 7 (0 ~ 7) - debounce filter depth
#define SHORT_TICKS             (300 / TICKS_INTERVAL)   // short press threshold
#define LONG_TICKS              (1000 / TICKS_INTERVAL)  // long press threshold
#define LONG_CALLBACK_TICKS		5	 // minimum TICKS interval between two BTN_LONG_PRESS_HOLD event callback. LONG_CALLBACK_TICKS * TICKS_INTERVAL = minimum interval in ms.
#define PRESS_REPEAT_MAX_NUM    15   // maximum repeat counter value

// Compile-time check: debounce_cnt is a 3-bit field, max value is 7
#if DEBOUNCE_TICKS > 7
  #error "DEBOUNCE_TICKS exceeds 3-bit field maximum (7)"
#endif
// Compile-time check: avoid div-0 problem
#if LONG_CALLBACK_TICKS < 1
  #error "LONG_CALLBACK_TICKS must be at least 1"
#endif


// Forward declaration
typedef struct _Button Button;

// Button callback function type
typedef void (*BtnCallback)(Button* handle, void* user_data);

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
	void*    user_data;                 // user context pointer passed to callbacks
	Button* next;                       // next button in linked list
};

// Optional thread-safety support for RTOS environments.
// Define MULTIBUTTON_THREAD_SAFE and provide MULTIBUTTON_LOCK()/MULTIBUTTON_UNLOCK()
// macros before including this header to enable thread-safe list operations.
//
// NOTE: Callbacks are executed OUTSIDE the lock, so a regular (non-recursive) mutex
// is safe. Callbacks may freely call button_stop()/button_start() without deadlock.
//
// Example:
//   #define MULTIBUTTON_THREAD_SAFE
//   #define MULTIBUTTON_LOCK()   osMutexAcquire(btn_mutex, osWaitForever)
//   #define MULTIBUTTON_UNLOCK() osMutexRelease(btn_mutex)
//   #include "multi_button.h"
#ifdef MULTIBUTTON_THREAD_SAFE
  #if !defined(MULTIBUTTON_LOCK) || !defined(MULTIBUTTON_UNLOCK)
    #error "Define MULTIBUTTON_LOCK() and MULTIBUTTON_UNLOCK() when using MULTIBUTTON_THREAD_SAFE"
  #endif
#else
  #define MULTIBUTTON_LOCK()
  #define MULTIBUTTON_UNLOCK()
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Public API functions
void button_init(Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id);
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb, void* user_data);
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
