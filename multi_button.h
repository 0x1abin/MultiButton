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

// Configuration constants. They may be overridden by compiler definitions.
#ifndef TICKS_INTERVAL
#define TICKS_INTERVAL          5U    // ms - legacy periodic timer interval
#endif
#ifndef DEBOUNCE_TICKS
#define DEBOUNCE_TICKS          3U    // debounce duration in legacy ticks
#endif
#ifndef SHORT_TICKS
#define SHORT_TICKS             (300U / TICKS_INTERVAL)
#endif
#ifndef LONG_TICKS
#define LONG_TICKS              (1000U / TICKS_INTERVAL)
#endif
#ifndef LONG_HOLD_TICKS
#define LONG_HOLD_TICKS         1U    // hold callback period in legacy ticks
#endif
#ifndef PRESS_REPEAT_MAX_NUM
#define PRESS_REPEAT_MAX_NUM    15U
#endif
#ifndef MULTIBUTTON_ENABLE_DOUBLE_CLICK
#define MULTIBUTTON_ENABLE_DOUBLE_CLICK 1
#endif

#if DEBOUNCE_TICKS < 1
  #error "DEBOUNCE_TICKS must be at least 1"
#endif
#if TICKS_INTERVAL < 1
  #error "TICKS_INTERVAL must be at least 1 ms"
#endif
#if MULTIBUTTON_ENABLE_DOUBLE_CLICK != 0 && MULTIBUTTON_ENABLE_DOUBLE_CLICK != 1
  #error "MULTIBUTTON_ENABLE_DOUBLE_CLICK must be 0 or 1"
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
	uint32_t tick_remainder_ms;         // elapsed time not forming a complete tick
	uint32_t debounce_elapsed_ms;       // time elapsed since a level change
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

/**
 * Process elapsed time and return the delay until the next required scan.
 *
 * Call this function from a one-shot timer and whenever a GPIO edge wakes the
 * system. elapsed_ms is the actual time since the previous call. A return value
 * of 0 means no timer is needed; scanning can remain stopped until a GPIO edge.
 */
uint32_t button_ticks_low_power(uint32_t elapsed_ms);

// Utility functions
uint8_t button_get_repeat_count(Button* handle);
void button_reset(Button* handle);
int button_is_pressed(Button* handle);

#ifdef __cplusplus
}
#endif

#endif
