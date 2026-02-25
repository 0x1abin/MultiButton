/* POSIX.1-2008 for usleep() under strict C99 */
#define _DEFAULT_SOURCE

/**
 * @file poll_example.c
 * @brief Demonstrates polling-mode button event detection (no callbacks).
 *
 * Instead of registering callbacks, this example calls button_get_event()
 * each tick to detect events in the main loop—useful for super-loop
 * architectures without interrupts.
 */

#include "multi_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static button_t  btn1;
static volatile int running = 1;

/* ----- Signal handler ----- */

static void on_sigint(int sig)
{
    (void)sig;
    printf("\nExiting polling example...\n");
    running = 0;
}

/* ----- HAL: predefined GPIO pattern for simulation ----- */

static uint8_t read_button_gpio(uint8_t button_id)
{
    (void)button_id;

    static int cycle, idx;

    static const int pattern[] = {
        0, 0, 0, 0, 0,                             /* idle         */
        1, 1, 1, 1, 0,                             /* short press  */
        0, 0, 0, 0, 0,                             /* idle         */
        1, 1, 0, 1, 1,                             /* double click */
        0, 0, 0, 0, 0,                             /* idle         */
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,   /* long press   */
        0, 0, 0, 0, 0,                             /* idle         */
        1, 0, 1, 0, 1, 0, 1, 0,                   /* rapid clicks */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0              /* final idle   */
    };
    static const int pattern_len = (int)(sizeof(pattern) / sizeof(pattern[0]));

    if (++cycle >= 10) {
        cycle = 0;
        idx   = (idx + 1) % pattern_len;
    }
    return (uint8_t)pattern[idx];
}

/* ----- Event name helper ----- */

static const char *event_name(button_event_t ev)
{
    switch (ev) {
    case BTN_PRESS_DOWN:       return "Press Down";
    case BTN_PRESS_UP:         return "Press Up";
    case BTN_SINGLE_CLICK:     return "Single Click";
    case BTN_DOUBLE_CLICK:     return "Double Click";
    case BTN_LONG_PRESS_START: return "Long Press Start";
    case BTN_LONG_PRESS_HOLD:  return "Long Press Hold";
    case BTN_PRESS_REPEAT:     return "Repeat";
    default:                   return "Unknown";
    }
}

/* ----- Poll loop ----- */

static void poll_events(void)
{
    static button_event_t last = BTN_NONE_PRESS;
    static int seq;

    button_event_t cur = button_get_event(&btn1);

    if (cur != last && cur != BTN_NONE_PRESS) {
        seq++;
        printf("  [%3d] %-18s | pressed=%s, repeat=%d, state=%d\n",
               seq, event_name(cur),
               button_is_pressed(&btn1) ? "yes" : "no",
               button_get_repeat_count(&btn1),
               button_get_state(&btn1));
        last = cur;
    }
}

static void print_status(void)
{
    static int counter;
    if (++counter < 200) return;
    counter = 0;

    printf("  status: pressed=%s, repeat=%d, event=%d, state=%d\n",
           button_is_pressed(&btn1) ? "yes" : "no",
           button_get_repeat_count(&btn1),
           button_get_event(&btn1),
           button_get_state(&btn1));
}

/* ----- Main ----- */

int main(void)
{
    printf("MultiButton Polling Example\n");
    printf("===========================\n\n");
    printf("Events are detected via button_get_event() (no callbacks).\n");
    printf("A predefined pattern simulates presses.\n\n");

    signal(SIGINT, on_sigint);

    button_init(&btn1, read_button_gpio, 1, 1);
    button_start(&btn1);
    printf("Button initialised for polling.\n\n");

    int ticks = 0;

    while (running) {
        button_tick();
        poll_events();
        print_status();
        usleep(BTN_TICKS_INTERVAL * 1000);

        if (++ticks > 2000) {
            printf("\nPattern completed.\n");
            break;
        }
    }

    button_stop(&btn1);

    printf("\nKey takeaways:\n");
    printf("  - Use button_get_event() to poll instead of callbacks\n");
    printf("  - button_tick() must still be called at %d ms intervals\n",
           BTN_TICKS_INTERVAL);
    printf("  - Ideal for bare-metal super-loop architectures\n");

    return 0;
}
