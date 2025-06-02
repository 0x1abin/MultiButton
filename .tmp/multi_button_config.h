/*
 * MultiButton Configuration Header
 * This file allows customization of button library behavior
 */

#ifndef _MULTI_BUTTON_CONFIG_H_
#define _MULTI_BUTTON_CONFIG_H_

// Timing configuration (in milliseconds)
#ifndef TICKS_INTERVAL
#define TICKS_INTERVAL          5       // Timer interrupt interval
#endif

#ifndef DEBOUNCE_TIME_MS
#define DEBOUNCE_TIME_MS        15      // Debounce time
#endif

#ifndef SHORT_PRESS_TIME_MS
#define SHORT_PRESS_TIME_MS     300     // Single/double click threshold
#endif

#ifndef LONG_PRESS_TIME_MS
#define LONG_PRESS_TIME_MS      1000    // Long press threshold
#endif

// Calculated tick values
#define DEBOUNCE_TICKS          (DEBOUNCE_TIME_MS / TICKS_INTERVAL)
#define SHORT_TICKS             (SHORT_PRESS_TIME_MS / TICKS_INTERVAL)
#define LONG_TICKS              (LONG_PRESS_TIME_MS / TICKS_INTERVAL)

// Functional configuration
#ifndef PRESS_REPEAT_MAX_NUM
#define PRESS_REPEAT_MAX_NUM    15      // Maximum repeat counter value
#endif

#ifndef ENABLE_TRIPLE_CLICK
#define ENABLE_TRIPLE_CLICK     0       // Enable triple click detection
#endif

#ifndef ENABLE_QUADRUPLE_CLICK
#define ENABLE_QUADRUPLE_CLICK  0       // Enable quadruple click detection
#endif

// Memory optimization
#ifndef BUTTON_USE_STATIC_ALLOCATION
#define BUTTON_USE_STATIC_ALLOCATION 1  // Use static allocation for buttons
#endif

#ifndef MAX_BUTTON_COUNT
#define MAX_BUTTON_COUNT        8       // Maximum number of buttons (if static allocation)
#endif

// Debug configuration
#ifndef BUTTON_DEBUG
#define BUTTON_DEBUG            0       // Enable debug output
#endif

// Validation macros
#if DEBOUNCE_TICKS > 7
#error "DEBOUNCE_TICKS must be <= 7 (limited by 3-bit field)"
#endif

#if PRESS_REPEAT_MAX_NUM > 15
#error "PRESS_REPEAT_MAX_NUM must be <= 15 (limited by 4-bit field)"
#endif

#endif // _MULTI_BUTTON_CONFIG_H_ 