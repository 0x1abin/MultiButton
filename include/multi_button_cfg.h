/**
 * @file multi_button_cfg.h
 * @brief Compile-time configuration for multi-button library
 *
 * Override any setting by:
 *   1. Defining it before including multi_button.h
 *   2. Defining BTN_USER_CFG_FILE as a string to include a custom config header
 *      e.g., -DBTN_USER_CFG_FILE=\"my_button_cfg.h\"
 *
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef MULTI_BUTTON_CFG_H
#define MULTI_BUTTON_CFG_H

#ifdef BTN_USER_CFG_FILE
#include BTN_USER_CFG_FILE
#endif

/* Timer tick interval in milliseconds.
 * Must match the actual period at which button_tick() is called. */
#ifndef BTN_TICKS_INTERVAL
#define BTN_TICKS_INTERVAL          5
#endif

/* Debounce filter depth in ticks (max 7, limited by 3-bit counter).
 * Debounce time = BTN_DEBOUNCE_TICKS * BTN_TICKS_INTERVAL ms */
#ifndef BTN_DEBOUNCE_TICKS
#define BTN_DEBOUNCE_TICKS          3
#endif

/* Short-press / multi-click window in ticks.
 * After releasing, the FSM waits this many ticks for another press
 * before committing to a single/double click event. */
#ifndef BTN_SHORT_TICKS
#define BTN_SHORT_TICKS             (300 / BTN_TICKS_INTERVAL)
#endif

/* Long-press threshold in ticks.
 * If a button is held longer than this, LONG_PRESS_START fires. */
#ifndef BTN_LONG_TICKS
#define BTN_LONG_TICKS              (1000 / BTN_TICKS_INTERVAL)
#endif

/* Maximum value of the repeat (multi-click) counter (max 15, 4-bit field). */
#ifndef BTN_REPEAT_MAX
#define BTN_REPEAT_MAX              15
#endif

#endif /* MULTI_BUTTON_CFG_H */
