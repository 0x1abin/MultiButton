/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#ifndef _MULTI_BUTTON_H_
#define _MULTI_BUTTON_H_

#include <stdint.h>
#include <string.h>

// Configuration constants - can be modified according to your needs
#define TICKS_INTERVAL 5                   // ms - timer interrupt interval
#define DEBOUNCE_TICKS 3                   // MAX 7 (0 ~ 7) - debounce filter depth
#define SHORT_TICKS (300 / TICKS_INTERVAL) // short press threshold
#define LONG_TICKS (1000 / TICKS_INTERVAL) // long press threshold
#define PRESS_REPEAT_MAX_NUM 15            // maximum repeat counter value

// Button event types
typedef enum
{
    BTN_PRESS_DOWN = 0,   // button pressed down
    BTN_PRESS_UP,         // button released
    BTN_PRESS_REPEAT,     // repeated press detected
    BTN_SINGLE_CLICK,     // single click completed
    BTN_DOUBLE_CLICK,     // double click completed
    BTN_LONG_PRESS_START, // long press started
    BTN_LONG_PRESS_HOLD,  // long press holding
    BTN_EVENT_COUNT,      // total number of events
    BTN_NONE_PRESS        // no event
} ButtonEvent;

// Button state machine states
typedef enum
{
    BTN_STATE_IDLE = 0, // idle state
    BTN_STATE_PRESS,    // pressed state
    BTN_STATE_RELEASE,  // released state waiting for timeout
    BTN_STATE_REPEAT,   // repeat press state
    BTN_STATE_LONG_HOLD // long press hold state
} ButtonState;


typedef void (*MultiButtonCallback)(uint16_t buttonId, ButtonEvent event, void *userData);

typedef struct
{
    uint16_t ticks;                                 // tick counter
    uint8_t repeat : 4;                             // repeat counter (0-15)
    uint8_t event : 4;                              // current event (0-15)
    uint8_t state : 3;                              // state machine state (0-7)
    uint8_t debounce_cnt : 3;                       // debounce counter (0-7)
    uint8_t active_level : 1;                       // active GPIO level (0 or 1)
    uint8_t button_level : 1;                       // current button level
    uint8_t buttonId;                               // button identifier
    uint8_t (*hal_button_level)(uint16_t buttonId); // HAL function to read GPIO
    MultiButtonCallback callback;                   // callback function array
    void *userData;                                 // user data pointer
} MultiButton;

typedef struct
{
    uint16_t ticksInterval;
    uint16_t debounceTicks;
    uint16_t shortTicks;
    uint16_t longTicks;
    uint16_t pressRepeatMaxNum;
    const MultiButton *const *buttonList;
    uint16_t buttonCount;
} MultiButtonConfig;

typedef struct
{
    MultiButtonConfig config;
} MultiButtonContext;

#ifdef __cplusplus
extern "C"
{
#endif

    int multiButtonInstall(MultiButtonConfig *config);

#ifdef __cplusplus
}
#endif

#endif
