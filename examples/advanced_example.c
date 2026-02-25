/* POSIX.1-2008 for usleep() under strict C99 */
#define _DEFAULT_SOURCE

/**
 * @file advanced_example.c
 * @brief Demonstrates advanced features: multi-button, dynamic callbacks,
 *        user_data context, and runtime reconfiguration.
 */

#include "multi_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define NUM_BUTTONS 4

/* ----- Application context carried via user_data ----- */

typedef struct {
    const char *label;
    int         click_total;
} btn_ctx_t;

/* ----- Globals ----- */

static button_t  buttons[NUM_BUTTONS];
static btn_ctx_t contexts[NUM_BUTTONS];
static int        gpio_states[NUM_BUTTONS];
static volatile int running = 1;
static int verbose;

/* ----- Signal handler ----- */

static void on_sigint(int sig)
{
    (void)sig;
    printf("\nSIGINT received, cleaning up...\n");
    running = 0;
}

/* ----- HAL ----- */

static uint8_t read_gpio(uint8_t button_id)
{
    if (button_id >= 1 && button_id <= NUM_BUTTONS)
        return (uint8_t)gpio_states[button_id - 1];
    return 0;
}

/* ----- Generic callback that uses user_data ----- */

static void on_event(button_t *btn)
{
    btn_ctx_t *ctx = (btn_ctx_t *)button_get_user_data(btn);
    const char *name = ctx ? ctx->label : "?";

    button_event_t ev = button_get_event(btn);
    const char *ev_str;
    switch (ev) {
    case BTN_PRESS_DOWN:       ev_str = "Press Down";       break;
    case BTN_PRESS_UP:         ev_str = "Press Up";         break;
    case BTN_PRESS_REPEAT:     ev_str = "Repeat";           break;
    case BTN_SINGLE_CLICK:     ev_str = "Single Click";     break;
    case BTN_DOUBLE_CLICK:     ev_str = "Double Click";     break;
    case BTN_LONG_PRESS_START: ev_str = "Long Press Start"; break;
    case BTN_LONG_PRESS_HOLD:  ev_str = "Long Press Hold";  break;
    default:                   ev_str = "Unknown";          break;
    }

    if (verbose) {
        printf("  [%s] %s  (repeat=%d, pressed=%s)\n",
               name, ev_str,
               button_get_repeat_count(btn),
               button_is_pressed(btn) ? "yes" : "no");
    } else {
        printf("  [%s] %s\n", name, ev_str);
    }

    if (ev == BTN_SINGLE_CLICK || ev == BTN_DOUBLE_CLICK) {
        if (ctx) ctx->click_total++;
    }
}

/* ----- Config-button special handler ----- */

static void on_config_click(button_t *btn)
{
    static int step;
    btn_ctx_t *ctx = (btn_ctx_t *)button_get_user_data(btn);
    printf("  [%s] Config action #%d\n", ctx ? ctx->label : "?", step);

    switch (step) {
    case 0:
        verbose = !verbose;
        printf("    -> verbose: %s\n", verbose ? "ON" : "OFF");
        break;
    case 1:
        printf("    -> resetting all buttons\n");
        for (int i = 0; i < NUM_BUTTONS; i++)
            button_reset(&buttons[i]);
        break;
    default:
        break;
    }
    step = (step + 1) % 3;
}

/* ----- Setup helpers ----- */

static void init_button(int idx, uint8_t id, const char *label, int all_events)
{
    button_init(&buttons[idx], read_gpio, 1, id);

    contexts[idx].label       = label;
    contexts[idx].click_total = 0;
    button_set_user_data(&buttons[idx], &contexts[idx]);

    if (all_events) {
        button_attach(&buttons[idx], BTN_PRESS_DOWN,       on_event);
        button_attach(&buttons[idx], BTN_PRESS_UP,         on_event);
        button_attach(&buttons[idx], BTN_PRESS_REPEAT,     on_event);
        button_attach(&buttons[idx], BTN_SINGLE_CLICK,     on_event);
        button_attach(&buttons[idx], BTN_DOUBLE_CLICK,     on_event);
        button_attach(&buttons[idx], BTN_LONG_PRESS_START, on_event);
        button_attach(&buttons[idx], BTN_LONG_PRESS_HOLD,  on_event);
    } else {
        button_attach(&buttons[idx], BTN_SINGLE_CLICK,     on_event);
        button_attach(&buttons[idx], BTN_DOUBLE_CLICK,     on_event);
        button_attach(&buttons[idx], BTN_LONG_PRESS_START, on_event);
    }

    button_start(&buttons[idx]);
}

static void buttons_setup(void)
{
    printf("Initialising %d buttons...\n", NUM_BUTTONS);

    init_button(0, 1, "BTN-A", 1);
    printf("  BTN-A : full event set\n");

    init_button(1, 2, "BTN-B", 0);
    printf("  BTN-B : essential events only\n");

    init_button(2, 3, "CONFIG", 0);
    button_detach(&buttons[2], BTN_SINGLE_CLICK);
    button_attach(&buttons[2], BTN_SINGLE_CLICK, on_config_click);
    printf("  CONFIG: special single-click handler\n");

    init_button(3, 4, "BTN-D", 0);
    printf("  BTN-D : dynamic reconfiguration demo\n");

    printf("All buttons ready.\n\n");
}

/* ----- Simulation ----- */

static void sim_tick(int ms)
{
    for (int i = 0; i < ms / BTN_TICKS_INTERVAL; i++) {
        button_tick();
        usleep(BTN_TICKS_INTERVAL * 1000);
    }
}

static void sim_press(int id, int duration_ms)
{
    if (id < 1 || id > NUM_BUTTONS) return;

    if (verbose)
        printf("\n-- sim btn%d %dms --\n", id, duration_ms);

    gpio_states[id - 1] = 1;
    sim_tick(duration_ms);

    gpio_states[id - 1] = 0;
    sim_tick(BTN_DEBOUNCE_TICKS * BTN_TICKS_INTERVAL + BTN_TICKS_INTERVAL);
}

/* ----- Demo sequences ----- */

static void demo_dynamic_reconfig(void)
{
    printf("\n=== Dynamic Reconfiguration ===\n");

    printf("1) BTN-D with minimal handlers:\n");
    sim_press(4, 100);
    sim_tick(400);

    printf("2) Adding press down/up handlers to BTN-D:\n");
    button_attach(&buttons[3], BTN_PRESS_DOWN,   on_event);
    button_attach(&buttons[3], BTN_PRESS_UP,     on_event);
    button_attach(&buttons[3], BTN_PRESS_REPEAT, on_event);
    sim_press(4, 100);
    sim_tick(400);

    printf("3) Removing press down/up from BTN-D:\n");
    button_detach(&buttons[3], BTN_PRESS_DOWN);
    button_detach(&buttons[3], BTN_PRESS_UP);
    sim_press(4, 100);
    sim_tick(400);
}

static void demo_main_sequence(void)
{
    printf("\n=== Main Demo Sequence ===\n");

    printf("\nSingle clicks on all buttons:\n");
    for (int i = 1; i <= NUM_BUTTONS; i++) {
        sim_press(i, 100);
        sim_tick(400);
    }

    printf("\nDouble click BTN-A:\n");
    sim_press(1, 80);
    sim_tick(50);
    sim_press(1, 80);
    sim_tick(400);

    printf("\nLong press BTN-B:\n");
    sim_press(2, 1200);
    sim_tick(200);

    printf("\nRapid presses BTN-A:\n");
    for (int i = 0; i < 4; i++) {
        sim_press(1, 60);
        sim_tick(70);
    }
    sim_tick(400);

    printf("\nConfig button clicks:\n");
    for (int i = 0; i < 3; i++) {
        sim_press(3, 100);
        sim_tick(400);
    }
}

/* ----- Status report ----- */

static void print_status(void)
{
    printf("\n=== Status Report ===\n");
    for (int i = 0; i < NUM_BUTTONS; i++) {
        btn_ctx_t *ctx = &contexts[i];
        printf("  %s : state=%d, pressed=%s, repeat=%d, clicks=%d\n",
               ctx->label,
               button_get_state(&buttons[i]),
               button_is_pressed(&buttons[i]) ? "yes" : "no",
               button_get_repeat_count(&buttons[i]),
               ctx->click_total);
    }
}

/* ----- Main ----- */

int main(int argc, char *argv[])
{
    int demo_mode = 1;

    printf("MultiButton Advanced Example\n");
    printf("============================\n");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            verbose = 1;
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            demo_mode = 0;
    }

    printf("Config: demo=%s, verbose=%s\n\n",
           demo_mode ? "ON" : "OFF", verbose ? "ON" : "OFF");

    signal(SIGINT, on_sigint);
    buttons_setup();

    if (demo_mode) {
        demo_main_sequence();
        demo_dynamic_reconfig();
        print_status();
        printf("\nAdvanced demo completed.\n");
    } else {
        printf("Manual test mode. Press Ctrl+C to exit.\n");
        while (running) {
            button_tick();
            usleep(BTN_TICKS_INTERVAL * 1000);
        }
    }

    for (int i = 0; i < NUM_BUTTONS; i++)
        button_stop(&buttons[i]);

    printf("Done.\n");
    return 0;
}
