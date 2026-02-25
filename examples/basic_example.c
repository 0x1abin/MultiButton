/* POSIX.1-2008 for usleep() under strict C99 */
#define _DEFAULT_SOURCE

/**
 * @file basic_example.c
 * @brief Demonstrates basic usage of the MultiButton library
 *
 * Covers: single click, double click, long press, repeat press, state query.
 * GPIO is simulated so this example runs on any host machine.
 */

#include "multi_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/* ----- Button instances (statically allocated) ----- */

static button_t btn1, btn2;
static volatile int running = 1;
static int gpio_btn1, gpio_btn2;

/* ----- Signal handler ----- */

static void on_sigint(int sig)
{
    (void)sig;
    printf("\nReceived SIGINT, exiting...\n");
    running = 0;
}

/* ----- HAL: simulated GPIO read ----- */

static uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id) {
    case 1:  return (uint8_t)gpio_btn1;
    case 2:  return (uint8_t)gpio_btn2;
    default: return 0;
    }
}

/* ----- Callback handlers for button 1 ----- */

static void btn1_on_single_click(button_t *btn)
{
    (void)btn;
    printf("  [btn1] Single Click\n");
}

static void btn1_on_double_click(button_t *btn)
{
    (void)btn;
    printf("  [btn1] Double Click\n");
}

static void btn1_on_long_press_start(button_t *btn)
{
    (void)btn;
    printf("  [btn1] Long Press Start\n");
}

static void btn1_on_long_press_hold(button_t *btn)
{
    (void)btn;
    printf("  [btn1] Long Press Hold...\n");
}

static void btn1_on_repeat(button_t *btn)
{
    printf("  [btn1] Repeat (count: %d)\n", button_get_repeat_count(btn));
}

/* ----- Callback handlers for button 2 ----- */

static void btn2_on_single_click(button_t *btn)
{
    (void)btn;
    printf("  [btn2] Single Click\n");
}

static void btn2_on_double_click(button_t *btn)
{
    (void)btn;
    printf("  [btn2] Double Click\n");
}

static void btn2_on_press_down(button_t *btn)
{
    (void)btn;
    printf("  [btn2] Press Down\n");
}

static void btn2_on_press_up(button_t *btn)
{
    (void)btn;
    printf("  [btn2] Press Up\n");
}

/* ----- Setup ----- */

static void buttons_setup(void)
{
    button_init(&btn1, read_button_gpio, 1, 1);
    button_attach(&btn1, BTN_SINGLE_CLICK,     btn1_on_single_click);
    button_attach(&btn1, BTN_DOUBLE_CLICK,     btn1_on_double_click);
    button_attach(&btn1, BTN_LONG_PRESS_START, btn1_on_long_press_start);
    button_attach(&btn1, BTN_LONG_PRESS_HOLD,  btn1_on_long_press_hold);
    button_attach(&btn1, BTN_PRESS_REPEAT,     btn1_on_repeat);
    button_start(&btn1);

    button_init(&btn2, read_button_gpio, 1, 2);
    button_attach(&btn2, BTN_SINGLE_CLICK, btn2_on_single_click);
    button_attach(&btn2, BTN_DOUBLE_CLICK, btn2_on_double_click);
    button_attach(&btn2, BTN_PRESS_DOWN,   btn2_on_press_down);
    button_attach(&btn2, BTN_PRESS_UP,     btn2_on_press_up);
    button_start(&btn2);
}

/* ----- Simulation helpers ----- */

static void sim_tick(int ms)
{
    for (int i = 0; i < ms / BTN_TICKS_INTERVAL; i++) {
        button_tick();
        usleep(BTN_TICKS_INTERVAL * 1000);
    }
}

static void sim_press(int id, int duration_ms)
{
    int *gpio = (id == 1) ? &gpio_btn1 : &gpio_btn2;

    printf("\n-- Simulating button %d press (%d ms) --\n", id, duration_ms);

    *gpio = 1;
    sim_tick(duration_ms);

    *gpio = 0;
    sim_tick(BTN_DEBOUNCE_TICKS * BTN_TICKS_INTERVAL + BTN_TICKS_INTERVAL);
}

/* ----- Main ----- */

int main(void)
{
    printf("MultiButton Basic Example\n");
    printf("=========================\n\n");

    signal(SIGINT, on_sigint);
    buttons_setup();
    printf("Buttons initialised.\n");

    printf("\n[1] Single Click Demo\n");
    sim_press(1, 100);
    sim_tick(400);

    printf("\n[2] Double Click Demo\n");
    sim_press(1, 100);
    sim_tick(50);
    sim_press(1, 100);
    sim_tick(400);

    printf("\n[3] Long Press Demo\n");
    sim_press(1, 1500);
    sim_tick(100);

    printf("\n[4] Repeat Press Demo (button 2)\n");
    for (int i = 0; i < 3; i++) {
        sim_press(2, 80);
        sim_tick(80);
    }
    sim_tick(400);

    printf("\n[5] State Query Demo\n");
    printf("  btn1 pressed: %s\n", button_is_pressed(&btn1) ? "yes" : "no");
    printf("  btn2 pressed: %s\n", button_is_pressed(&btn2) ? "yes" : "no");
    printf("  btn1 repeat count: %d\n", button_get_repeat_count(&btn1));
    printf("  btn2 repeat count: %d\n", button_get_repeat_count(&btn2));
    printf("  btn1 state: %d\n", button_get_state(&btn1));

    printf("\nDemo completed.\n");
    return 0;
}
