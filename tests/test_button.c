/*
 * MultiButton Unit Tests
 * Minimal test framework with no external dependencies.
 * Uses a mock GPIO function to drive the state machine deterministically.
 */

#include "multi_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Minimal test framework ---- */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(expr) do { \
    if (!(expr)) { \
        printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
        return 1; \
    } \
} while(0)

#define RUN_TEST(fn) do { \
    tests_run++; \
    printf("  [%d] %s ... ", tests_run, #fn); \
    if (fn() == 0) { tests_passed++; printf("OK\n"); } \
    else { tests_failed++; printf("FAILED\n"); } \
} while(0)

/* ---- Mock GPIO ---- */
static uint8_t mock_gpio_value = 0;

static uint8_t mock_read_gpio(uint8_t button_id)
{
    (void)button_id;
    return mock_gpio_value;
}

/* ---- Helper: advance N ticks ---- */
static void tick_n(int n)
{
    for (int i = 0; i < n; i++) {
        button_ticks();
    }
}

/* ---- Event tracking via callbacks ---- */
#define MAX_EVENTS 64
static ButtonEvent event_log[MAX_EVENTS];
static int event_count = 0;

static void reset_event_log(void) { event_count = 0; }

static void log_press_down(Button* btn)    { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_PRESS_DOWN; }
static void log_press_up(Button* btn)      { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_PRESS_UP; }
static void log_single_click(Button* btn)  { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_SINGLE_CLICK; }
static void log_double_click(Button* btn)  { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_DOUBLE_CLICK; }
static void log_long_start(Button* btn)    { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_LONG_PRESS_START; }
static void log_long_hold(Button* btn)     { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_LONG_PRESS_HOLD; }
static void log_repeat(Button* btn)        { (void)btn; if (event_count < MAX_EVENTS) event_log[event_count++] = BTN_PRESS_REPEAT; }

static int has_event(ButtonEvent ev)
{
    for (int i = 0; i < event_count; i++) {
        if (event_log[i] == ev) return 1;
    }
    return 0;
}

static int count_event(ButtonEvent ev)
{
    int c = 0;
    for (int i = 0; i < event_count; i++) {
        if (event_log[i] == ev) c++;
    }
    return c;
}

/* ---- Helper: init button with all logging callbacks ---- */
static Button test_btn;

static void setup_button(void)
{
    mock_gpio_value = 0;
    reset_event_log();
    button_init(&test_btn, mock_read_gpio, 1, 1);
    button_attach(&test_btn, BTN_PRESS_DOWN, log_press_down);
    button_attach(&test_btn, BTN_PRESS_UP, log_press_up);
    button_attach(&test_btn, BTN_SINGLE_CLICK, log_single_click);
    button_attach(&test_btn, BTN_DOUBLE_CLICK, log_double_click);
    button_attach(&test_btn, BTN_LONG_PRESS_START, log_long_start);
    button_attach(&test_btn, BTN_LONG_PRESS_HOLD, log_long_hold);
    button_attach(&test_btn, BTN_PRESS_REPEAT, log_repeat);
    button_start(&test_btn);
}

static void teardown_button(void)
{
    button_stop(&test_btn);
    mock_gpio_value = 0;
}

/* ============================================================
 * Test cases
 * ============================================================ */

/* Test 1: Single click */
static int test_single_click(void)
{
    setup_button();

    /* Press for ~50ms (10 ticks), then release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + SHORT_TICKS + 10);  /* wait for timeout */

    ASSERT(has_event(BTN_PRESS_DOWN));
    ASSERT(has_event(BTN_PRESS_UP));
    ASSERT(has_event(BTN_SINGLE_CLICK));
    ASSERT(!has_event(BTN_DOUBLE_CLICK));
    ASSERT(!has_event(BTN_LONG_PRESS_START));

    teardown_button();
    return 0;
}

/* Test 2: Double click */
static int test_double_click(void)
{
    setup_button();

    /* First click */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 5);

    /* Second click (within SHORT_TICKS) */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + SHORT_TICKS + 10);

    ASSERT(has_event(BTN_DOUBLE_CLICK));
    ASSERT(count_event(BTN_PRESS_DOWN) == 2);

    teardown_button();
    return 0;
}

/* Test 3: Long press */
static int test_long_press(void)
{
    setup_button();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_TICKS + 20);

    ASSERT(has_event(BTN_PRESS_DOWN));
    ASSERT(has_event(BTN_LONG_PRESS_START));
    ASSERT(has_event(BTN_LONG_PRESS_HOLD));
    ASSERT(count_event(BTN_LONG_PRESS_START) == 1);  /* exactly once */

    /* Release */
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 10);
    ASSERT(has_event(BTN_PRESS_UP));

    teardown_button();
    return 0;
}

/* Test 4: Repeat press count */
static int test_repeat_press(void)
{
    setup_button();

    for (int i = 0; i < 3; i++) {
        mock_gpio_value = 1;
        tick_n(DEBOUNCE_TICKS + 8);
        mock_gpio_value = 0;
        tick_n(DEBOUNCE_TICKS + 5);
    }

    /* Wait for timeout */
    tick_n(SHORT_TICKS + 20);

    ASSERT(count_event(BTN_PRESS_DOWN) == 3);
    ASSERT(has_event(BTN_PRESS_REPEAT));

    teardown_button();
    return 0;
}

/* Test 5: Debounce rejects noise */
static int test_debounce(void)
{
    setup_button();

    /* Toggle GPIO rapidly (less than DEBOUNCE_TICKS consecutive readings) */
    for (int i = 0; i < 20; i++) {
        mock_gpio_value = (i % 2);  /* alternate every tick */
        tick_n(1);
    }
    mock_gpio_value = 0;
    tick_n(10);

    ASSERT(!has_event(BTN_PRESS_DOWN));
    ASSERT(!has_event(BTN_SINGLE_CLICK));

    teardown_button();
    return 0;
}

/* Test 6: REPEAT→PRESS transition resets ticks (bug fix verification) */
static int test_repeat_to_press_transition(void)
{
    setup_button();

    /* First click: quick press and release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 8);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 5);

    /* Second click: press and hold past SHORT_TICKS into REPEAT, then stay held */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + SHORT_TICKS + 20);

    /* At this point, should have transitioned REPEAT→PRESS with ticks reset.
     * It should NOT have fired BTN_LONG_PRESS_START yet because ticks was reset. */
    ASSERT(!has_event(BTN_LONG_PRESS_START));

    /* Continue holding for LONG_TICKS more to trigger long press */
    tick_n(LONG_TICKS + 10);
    ASSERT(has_event(BTN_LONG_PRESS_START));

    teardown_button();
    return 0;
}

/* Test 7: button_start duplicate returns -1 */
static int test_start_duplicate(void)
{
    setup_button();
    int ret = button_start(&test_btn);
    ASSERT(ret == -1);
    teardown_button();
    return 0;
}

/* Test 8: button_stop then restart works cleanly */
static int test_stop_and_restart(void)
{
    setup_button();

    button_stop(&test_btn);
    reset_event_log();

    /* Ticks should not process the stopped button */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + SHORT_TICKS + 10);
    ASSERT(!has_event(BTN_PRESS_DOWN));

    /* Restart and verify it works */
    button_reset(&test_btn);
    button_start(&test_btn);
    reset_event_log();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + SHORT_TICKS + 10);
    ASSERT(has_event(BTN_SINGLE_CLICK));

    teardown_button();
    return 0;
}

/* Test 9: NULL handle safety */
static int test_null_handle(void)
{
    /* These should not crash */
    button_init(NULL, mock_read_gpio, 1, 1);
    button_attach(NULL, BTN_SINGLE_CLICK, log_single_click);
    button_detach(NULL, BTN_SINGLE_CLICK);
    button_stop(NULL);
    button_reset(NULL);

    ASSERT(button_get_event(NULL) == BTN_NONE_PRESS);
    ASSERT(button_get_repeat_count(NULL) == 0);
    ASSERT(button_is_pressed(NULL) == -1);
    ASSERT(button_start(NULL) == -2);

    return 0;
}

/* Test 10: button_reset clears state properly */
static int test_reset(void)
{
    setup_button();

    /* Generate some activity */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);

    ASSERT(has_event(BTN_PRESS_DOWN));

    button_reset(&test_btn);
    ASSERT(button_get_event(&test_btn) == BTN_NONE_PRESS);
    ASSERT(button_get_repeat_count(&test_btn) == 0);

    teardown_button();
    return 0;
}

/* ============================================================ */

int main(void)
{
    printf("MultiButton Unit Tests\n");
    printf("======================\n");

    RUN_TEST(test_single_click);
    RUN_TEST(test_double_click);
    RUN_TEST(test_long_press);
    RUN_TEST(test_repeat_press);
    RUN_TEST(test_debounce);
    RUN_TEST(test_repeat_to_press_transition);
    RUN_TEST(test_start_duplicate);
    RUN_TEST(test_stop_and_restart);
    RUN_TEST(test_null_handle);
    RUN_TEST(test_reset);

    printf("\nResults: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf(", %d FAILED", tests_failed);
    }
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}
