/**
 * @file multi_button.h
 * @brief Lightweight multi-button state machine library for embedded systems
 * @version 2.0.0
 *
 * Features:
 *   - 4-state FSM: IDLE → PRESSED → RELEASED → LONG_HOLD
 *   - Hardware debounce via digital filter
 *   - Single click, double click, N-click, long press, repeat detection
 *   - Event callback + polling dual-mode
 *   - User context pointer (user_data) for OOP-style usage
 *   - Compile-time configurable via multi_button_cfg.h
 *   - Zero dynamic allocation, ISR-safe tick processing
 *
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef MULTI_BUTTON_H
#define MULTI_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "multi_button_cfg.h"

/* ---------------- Version ------------------------------------------------ */

#define MULTI_BUTTON_VERSION_MAJOR  2
#define MULTI_BUTTON_VERSION_MINOR  0
#define MULTI_BUTTON_VERSION_PATCH  0

/* ---------------- Types -------------------------------------------------- */

typedef struct button button_t;

/**
 * HAL callback: read the raw GPIO level of a button.
 * @param button_id  Identifier passed during button_init().
 * @return           0 or 1 representing the current pin level.
 */
typedef uint8_t (*button_read_pin_fn)(uint8_t button_id);

/**
 * User event callback.
 * @param btn  Pointer to the button that triggered the event.
 */
typedef void (*button_cb_fn)(button_t *btn);

/** Button events produced by the state machine. */
typedef enum {
    BTN_PRESS_DOWN = 0,         /**< Physical press detected (after debounce) */
    BTN_PRESS_UP,               /**< Physical release detected */
    BTN_PRESS_REPEAT,           /**< Repeated press in a multi-click sequence */
    BTN_SINGLE_CLICK,           /**< Single click completed (after timeout) */
    BTN_DOUBLE_CLICK,           /**< Double click completed (after timeout) */
    BTN_LONG_PRESS_START,       /**< Long press threshold reached */
    BTN_LONG_PRESS_HOLD,        /**< Continuous hold after long press start */
    BTN_EVENT_COUNT,            /**< Number of distinct events (array size) */
    BTN_NONE_PRESS              /**< No event / idle sentinel */
} button_event_t;

/** Internal FSM states (4-state machine). */
typedef enum {
    BTN_STATE_IDLE = 0,         /**< Waiting for press */
    BTN_STATE_PRESSED,          /**< Button held down, timing for long press */
    BTN_STATE_RELEASED,         /**< Released, waiting for re-press or timeout */
    BTN_STATE_LONG_HOLD,        /**< Long press active, firing hold events */
    BTN_STATE_COUNT
} button_state_t;

/**
 * Button instance.
 *
 * Allocate statically (global / stack) or from a pool—never malloc in ISR.
 * Internal fields are documented for debugging; do not modify directly.
 */
struct button {
    /* --- Timing --- */
    uint32_t            ticks;          /**< Tick counter (overflow-safe) */

    /* --- Packed state (3 bytes total) --- */
    uint8_t             repeat       : 4;   /**< Multi-click counter 0–15 */
    uint8_t             event        : 4;   /**< Current event id */
    uint8_t             state        : 3;   /**< FSM state */
    uint8_t             debounce_cnt : 3;   /**< Debounce filter counter 0–7 */
    uint8_t             active_level : 1;   /**< GPIO level when pressed */
    uint8_t             button_level : 1;   /**< Debounced logical level */

    /* --- Identity --- */
    uint8_t             id;                 /**< User-assigned button identifier */

    /* --- HAL --- */
    button_read_pin_fn  hal_read_level;     /**< GPIO read function pointer */

    /* --- User context --- */
    void               *user_data;          /**< Opaque pointer for user data */

    /* --- Callbacks --- */
    button_cb_fn        cb[BTN_EVENT_COUNT]; /**< Per-event callback table */

    /* --- Linked list --- */
    button_t           *next;               /**< Next button in tick list */
};

/* ---------------- Initialization ----------------------------------------- */

/**
 * Initialize a button instance.
 *
 * @param btn           Pointer to a caller-allocated button_t.
 * @param read_fn       HAL function that returns the raw GPIO level.
 * @param active_level  GPIO level when the button is physically pressed (0 or 1).
 * @param id            Unique button identifier passed back to read_fn / callbacks.
 */
void button_init(button_t *btn, button_read_pin_fn read_fn,
                 uint8_t active_level, uint8_t id);

/**
 * Attach an opaque user-data pointer to the button instance.
 * Retrievable in callbacks via button_get_user_data().
 */
void button_set_user_data(button_t *btn, void *user_data);

/**
 * Retrieve the user-data pointer previously set with button_set_user_data().
 */
void *button_get_user_data(const button_t *btn);

/* ---------------- Event callback management ------------------------------ */

/**
 * Register a callback for a specific event.
 * Replaces any previously registered callback for the same event.
 */
void button_attach(button_t *btn, button_event_t event, button_cb_fn cb);

/**
 * Remove the callback for a specific event (sets it to NULL).
 */
void button_detach(button_t *btn, button_event_t event);

/* ---------------- Lifecycle ---------------------------------------------- */

/**
 * Add a button to the global tick list.
 *
 * @return  0 on success, -1 if already registered, -2 on invalid param.
 */
int button_start(button_t *btn);

/**
 * Remove a button from the global tick list.
 */
void button_stop(button_t *btn);

/* ---------------- Tick processing ---------------------------------------- */

/**
 * Process all registered buttons (call from timer ISR or RTOS tick).
 * Must be invoked at the interval defined by BTN_TICKS_INTERVAL.
 */
void button_tick(void);

/**
 * Process a single button instance.
 * Use this when managing your own button collection instead of the
 * built-in linked list (e.g., iterating an array in a specific task).
 */
void button_tick_single(button_t *btn);

/* ---------------- Query -------------------------------------------------- */

/** Get the most recent event for polling-mode usage. */
button_event_t button_get_event(const button_t *btn);

/** Get the current FSM state (mainly for debugging). */
button_state_t button_get_state(const button_t *btn);

/** Get the multi-click repeat counter. */
uint8_t button_get_repeat_count(const button_t *btn);

/** Get the button identifier. */
uint8_t button_get_id(const button_t *btn);

/** Return true if the debounced level equals the active level. */
bool button_is_pressed(const button_t *btn);

/* ---------------- Utility ------------------------------------------------ */

/** Reset button state to IDLE, clearing counters and event. */
void button_reset(button_t *btn);

#ifdef __cplusplus
}
#endif

#endif /* MULTI_BUTTON_H */
