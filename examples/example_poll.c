#include "multi_button.h"

unit8_t btn1_id = 0;
struct Button btn1;

uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case btn1_id:
			return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
		default:
			return 0;
	}
}


int main()
{
	static uint8_t btn1_event_val;

	button_init(&btn1, read_button_GPIO, 0, btn1_id);
	button_start(&btn1);

	//make the timer invoking the button_ticks() interval 5ms.
	//This function is implemented by yourself.
	__timer_start(button_ticks, 0, 5);

	while(1)
	{
		if(btn1_event_val != get_button_event(&btn1)) {
			btn1_event_val = get_button_event(&btn1);

			if(btn1_event_val == PRESS_DOWN) {
				//do something
			} else if(btn1_event_val == PRESS_UP) {
				//do something
			} else if(btn1_event_val == LONG_PRESS_HOLD) {
				//do something
			}
		}
	}
}
