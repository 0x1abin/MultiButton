#include "multi_button.h"

#include <stdio.h>

#if MULTIBUTTON_ENABLE_DOUBLE_CLICK
#error "This test must be compiled with MULTIBUTTON_ENABLE_DOUBLE_CLICK=0"
#endif

static uint8_t gpio_level;
static unsigned int press_up_count;
static unsigned int single_click_count;
static unsigned int double_click_count;

static uint8_t read_gpio(uint8_t button_id)
{
	(void)button_id;
	return gpio_level;
}

static void on_press_up(Button* button, void* user_data)
{
	(void)button;
	(void)user_data;
	press_up_count++;
}

static void on_single_click(Button* button, void* user_data)
{
	(void)button;
	(void)user_data;
	single_click_count++;
}

static void on_double_click(Button* button, void* user_data)
{
	(void)button;
	(void)user_data;
	double_click_count++;
}

int main(void)
{
	Button button;
	uint32_t next;

	gpio_level = 0;
	button_init(&button, read_gpio, 1, 0);
	button_attach(&button, BTN_PRESS_UP, on_press_up, NULL);
	button_attach(&button, BTN_SINGLE_CLICK, on_single_click, NULL);
	button_attach(&button, BTN_DOUBLE_CLICK, on_double_click, NULL);
	if (button_start(&button) != 0) {
		return 1;
	}

	gpio_level = 1;
	next = button_ticks_low_power(0);
	if (next != DEBOUNCE_TICKS * TICKS_INTERVAL) {
		return 2;
	}
	next = button_ticks_low_power(next);
	if (next != LONG_TICKS * TICKS_INTERVAL) {
		return 3;
	}

	gpio_level = 0;
	next = button_ticks_low_power(25);
	if (next != DEBOUNCE_TICKS * TICKS_INTERVAL) {
		return 4;
	}
	next = button_ticks_low_power(next);

	if (press_up_count != 1 || single_click_count != 1 ||
	    double_click_count != 0 || next != 0) {
		return 5;
	}

	button_stop(&button);
	printf("Double-click disabled test passed\n");
	return 0;
}
