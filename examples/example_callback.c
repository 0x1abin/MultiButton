#include "multi_button.h"

struct Button btn1;
struct Button btn2;

uint8_t read_button1_GPIO() 
{
	return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
}

uint8_t read_button2_GPIO() 
{
	return HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin);
}

int main()
{
	button_init(&btn1, read_button1_GPIO, 0);
	button_init(&btn2, read_button2_GPIO, 0);
	
	button_attach(&btn1, PRESS_DOWN,       BTN1_PRESS_DOWN_Handler);
	button_attach(&btn1, PRESS_UP,         BTN1_PRESS_UP_Handler);
	button_attach(&btn1, PRESS_REPEAT,     BTN1_PRESS_REPEAT_Handler);
	button_attach(&btn1, SINGLE_CLICK,     BTN1_SINGLE_Click_Handler);
	button_attach(&btn1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
	button_attach(&btn1, LONG_RRESS_START, BTN1_LONG_RRESS_START_Handler);
	button_attach(&btn1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);
	
	button_attach(&btn2, PRESS_DOWN,       BTN2_PRESS_DOWN_Handler);
	button_attach(&btn2, PRESS_UP,         BTN2_PRESS_UP_Handler);
	button_attach(&btn2, PRESS_REPEAT,     BTN2_PRESS_REPEAT_Handler);
	button_attach(&btn2, SINGLE_CLICK,     BTN2_SINGLE_Click_Handler);
	button_attach(&btn2, DOUBLE_CLICK,     BTN2_DOUBLE_Click_Handler);
	button_attach(&btn2, LONG_RRESS_START, BTN2_LONG_RRESS_START_Handler);
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

...