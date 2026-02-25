/**
 * @file multi_button.c
 * @brief Multi-button state machine implementation
 *
 * 4-state FSM design:
 *
 *   [IDLE] ──press──▶ [PRESSED] ──long hold──▶ [LONG_HOLD]
 *     ▲                  │                          │
 *     │               release                    release
 *     │                  ▼                          │
 *     └──timeout──── [RELEASED] ◀───────────────────┘
 *                     │      ▲
 *                 re-press   │
 *                     └──▶ [PRESSED]
 *
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * SPDX-License-Identifier: MIT
 */

#include "multi_button.h"
#include <string.h>

/* Fire a callback if registered. */
#define FIRE_CB(btn, ev) \
    do { if ((btn)->cb[ev]) (btn)->cb[ev](btn); } while (0)

/* Global linked list of buttons registered via button_start(). */
static button_t *btn_list_head = NULL;

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

static inline uint8_t read_pin(const button_t *btn)
{
    return btn->hal_read_level(btn->id);
}

/**
 * Digital debounce filter.
 * The debounced level (button_level) only changes after the raw input
 * has been stable for BTN_DEBOUNCE_TICKS consecutive samples.
 */
static inline void debounce_filter(button_t *btn, uint8_t raw)
{
    if (raw != btn->button_level) {
        if (++(btn->debounce_cnt) >= BTN_DEBOUNCE_TICKS) {
            btn->button_level = raw;
            btn->debounce_cnt = 0;
        }
    } else {
        btn->debounce_cnt = 0;
    }
}

/* ------------------------------------------------------------------ */
/*  Core finite state machine                                          */
/* ------------------------------------------------------------------ */

static void button_fsm(button_t *btn)
{
    uint8_t raw = read_pin(btn);
    debounce_filter(btn, raw);

    uint8_t pressed = (btn->button_level == btn->active_level);

    switch ((button_state_t)btn->state) {

    /* ---- IDLE: waiting for first press ---- */
    case BTN_STATE_IDLE:
        if (pressed) {
            btn->event  = (uint8_t)BTN_PRESS_DOWN;
            FIRE_CB(btn, BTN_PRESS_DOWN);
            btn->ticks  = 0;
            btn->repeat = 1;
            btn->state  = (uint8_t)BTN_STATE_PRESSED;
        } else {
            btn->event = (uint8_t)BTN_NONE_PRESS;
        }
        break;

    /* ---- PRESSED: button is held, check for release or long-press ---- */
    case BTN_STATE_PRESSED:
        btn->ticks++;
        if (!pressed) {
            btn->event = (uint8_t)BTN_PRESS_UP;
            FIRE_CB(btn, BTN_PRESS_UP);
            btn->ticks = 0;
            btn->state = (uint8_t)BTN_STATE_RELEASED;
        } else if (btn->ticks > BTN_LONG_TICKS) {
            btn->event = (uint8_t)BTN_LONG_PRESS_START;
            FIRE_CB(btn, BTN_LONG_PRESS_START);
            btn->state = (uint8_t)BTN_STATE_LONG_HOLD;
        }
        break;

    /* ---- RELEASED: waiting for re-press (multi-click) or timeout ---- */
    case BTN_STATE_RELEASED:
        btn->ticks++;
        if (pressed) {
            btn->event = (uint8_t)BTN_PRESS_DOWN;
            FIRE_CB(btn, BTN_PRESS_DOWN);
            if (btn->repeat < BTN_REPEAT_MAX) {
                btn->repeat++;
            }
            btn->event = (uint8_t)BTN_PRESS_REPEAT;
            FIRE_CB(btn, BTN_PRESS_REPEAT);
            btn->ticks = 0;
            btn->state = (uint8_t)BTN_STATE_PRESSED;
        } else if (btn->ticks > BTN_SHORT_TICKS) {
            if (btn->repeat == 1) {
                btn->event = (uint8_t)BTN_SINGLE_CLICK;
                FIRE_CB(btn, BTN_SINGLE_CLICK);
            } else if (btn->repeat == 2) {
                btn->event = (uint8_t)BTN_DOUBLE_CLICK;
                FIRE_CB(btn, BTN_DOUBLE_CLICK);
            }
            btn->state = (uint8_t)BTN_STATE_IDLE;
        }
        break;

    /* ---- LONG_HOLD: button still held after long-press threshold ---- */
    case BTN_STATE_LONG_HOLD:
        if (pressed) {
            btn->event = (uint8_t)BTN_LONG_PRESS_HOLD;
            FIRE_CB(btn, BTN_LONG_PRESS_HOLD);
        } else {
            btn->event = (uint8_t)BTN_PRESS_UP;
            FIRE_CB(btn, BTN_PRESS_UP);
            btn->state = (uint8_t)BTN_STATE_IDLE;
        }
        break;

    default:
        btn->state = (uint8_t)BTN_STATE_IDLE;
        break;
    }
}

/* ------------------------------------------------------------------ */
/*  Public API: Initialization                                         */
/* ------------------------------------------------------------------ */

void button_init(button_t *btn, button_read_pin_fn read_fn,
                 uint8_t active_level, uint8_t id)
{
    if (!btn || !read_fn) return;

    memset(btn, 0, sizeof(button_t));
    btn->event          = (uint8_t)BTN_NONE_PRESS;
    btn->hal_read_level = read_fn;
    btn->button_level   = !active_level;
    btn->active_level   = active_level;
    btn->id             = id;
    btn->state          = (uint8_t)BTN_STATE_IDLE;
}

void button_set_user_data(button_t *btn, void *user_data)
{
    if (!btn) return;
    btn->user_data = user_data;
}

void *button_get_user_data(const button_t *btn)
{
    if (!btn) return NULL;
    return btn->user_data;
}

/* ------------------------------------------------------------------ */
/*  Public API: Event callback management                              */
/* ------------------------------------------------------------------ */

void button_attach(button_t *btn, button_event_t event, button_cb_fn cb)
{
    if (!btn || event >= BTN_EVENT_COUNT) return;
    btn->cb[event] = cb;
}

void button_detach(button_t *btn, button_event_t event)
{
    if (!btn || event >= BTN_EVENT_COUNT) return;
    btn->cb[event] = NULL;
}

/* ------------------------------------------------------------------ */
/*  Public API: Lifecycle                                              */
/* ------------------------------------------------------------------ */

int button_start(button_t *btn)
{
    if (!btn) return -2;

    /* Check for duplicates. */
    for (button_t *cur = btn_list_head; cur; cur = cur->next) {
        if (cur == btn) return -1;
    }

    btn->next     = btn_list_head;
    btn_list_head = btn;
    return 0;
}

void button_stop(button_t *btn)
{
    if (!btn) return;

    button_t **pp = &btn_list_head;
    while (*pp) {
        if (*pp == btn) {
            *pp       = btn->next;
            btn->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

/* ------------------------------------------------------------------ */
/*  Public API: Tick processing                                        */
/* ------------------------------------------------------------------ */

void button_tick(void)
{
    for (button_t *cur = btn_list_head; cur; cur = cur->next) {
        button_fsm(cur);
    }
}

void button_tick_single(button_t *btn)
{
    if (!btn) return;
    button_fsm(btn);
}

/* ------------------------------------------------------------------ */
/*  Public API: Query                                                  */
/* ------------------------------------------------------------------ */

button_event_t button_get_event(const button_t *btn)
{
    if (!btn) return BTN_NONE_PRESS;
    return (button_event_t)(btn->event);
}

button_state_t button_get_state(const button_t *btn)
{
    if (!btn) return BTN_STATE_IDLE;
    return (button_state_t)(btn->state);
}

uint8_t button_get_repeat_count(const button_t *btn)
{
    if (!btn) return 0;
    return btn->repeat;
}

uint8_t button_get_id(const button_t *btn)
{
    if (!btn) return 0;
    return btn->id;
}

bool button_is_pressed(const button_t *btn)
{
    if (!btn) return false;
    return (btn->button_level == btn->active_level);
}

/* ------------------------------------------------------------------ */
/*  Public API: Utility                                                */
/* ------------------------------------------------------------------ */

void button_reset(button_t *btn)
{
    if (!btn) return;
    btn->state        = (uint8_t)BTN_STATE_IDLE;
    btn->ticks        = 0;
    btn->repeat       = 0;
    btn->event        = (uint8_t)BTN_NONE_PRESS;
    btn->debounce_cnt = 0;
}
