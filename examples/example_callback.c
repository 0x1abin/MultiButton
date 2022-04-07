#include "multi_button.h"

enum Button_IDs {
	btn1_id,
	btn2_id,
};

struct Button btn1;
struct Button btn2;

uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case btn1_id:
			return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
		case btn2_id:
			return HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin);
		default:
			return 0;
	}
}

int main()
{
	button_init(&btn1, read_button_GPIO, 0, btn1_id);
	button_init(&btn2, read_button_GPIO, 0, btn2_id);

	button_attach(&btn1, PRESS_DOWN,       BTN1_PRESS_DOWN_Handler);
	button_attach(&btn1, PRESS_UP,         BTN1_PRESS_UP_Handler);
	button_attach(&btn1, PRESS_REPEAT,     BTN1_PRESS_REPEAT_Handler);
	button_attach(&btn1, SINGLE_CLICK,     BTN1_SINGLE_Click_Handler);
	button_attach(&btn1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
	button_attach(&btn1, LONG_PRESS_START, BTN1_LONG_PRESS_START_Handler);
	button_attach(&btn1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);

	button_attach(&btn2, PRESS_DOWN,       BTN2_PRESS_DOWN_Handler);
	button_attach(&btn2, PRESS_UP,         BTN2_PRESS_UP_Handler);
	button_attach(&btn2, PRESS_REPEAT,     BTN2_PRESS_REPEAT_Handler);
	button_attach(&btn2, SINGLE_CLICK,     BTN2_SINGLE_Click_Handler);
	button_attach(&btn2, DOUBLE_CLICK,     BTN2_DOUBLE_Click_Handler);
	button_attach(&btn2, LONG_PRESS_START, BTN2_LONG_PRESS_START_Handler);
	button_attach(&btn2, LONG_PRESS_HOLD,  BTN2_LONG_PRESS_HOLD_Handler);

	button_start(&btn1);
	button_start(&btn2);

	//make the timer invoking the button_ticks() interval 5ms.
	//This function is implemented by yourself.
	__timer_start(button_ticks, 0, 5);

	while(1)
	{}
}

void BTN1_PRESS_DOWN_Handler(void* btn)
{
	//do something...
}

void BTN1_PRESS_UP_Handler(void* btn)
{
	//do something...
}
