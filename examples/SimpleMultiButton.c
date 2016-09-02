#include "button.h"

struct Button btn1;
struct Button btn2;
struct Button btn3;

void main()
{
	button_init(&btn1, read_K1_pin, 0);
	button_attach(&btn1, PRESSED,          BTN1_PRESSED_Handler);
	button_attach(&btn1, CLICK,            BTN1_Click_Handler);
	button_attach(&btn1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
	button_attach(&btn1, LONG_RRESS_START, BTN1_LONG_RRESS_START_Handler);
	button_attach(&btn1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);
	button_attach(&btn1, LONG_PRESS_STOP,  BTN1_LONG_PRESS_STOP_Handler);
	button_start(&btn1);
	
	button_init(&btn2, read_K2_pin, 0);
	button_attach(&btn2, PRESSED,          BTN2_PRESSED_Handler);
	button_attach(&btn2, CLICK,            BTN2_Click_Handler);
	button_attach(&btn2, DOUBLE_CLICK,     BTN2_DOUBLE_Click_Handler);
	button_attach(&btn2, LONG_RRESS_START, BTN2_LONG_RRESS_START_Handler);
	button_attach(&btn2, LONG_PRESS_HOLD,  BTN2_LONG_PRESS_HOLD_Handler);
	button_attach(&btn2, LONG_PRESS_STOP,  BTN2_LONG_PRESS_STOP_Handler);
	button_start(&btn2);
	
	button_init(&btn3, read_K3_pin, 0);
	button_attach(&btn3, PRESSED,          BTN3_PRESSED_Handler);
	button_attach(&btn3, CLICK,            BTN3_Click_Handler);
	button_attach(&btn3, DOUBLE_CLICK,     BTN3_DOUBLE_Click_Handler);
	button_attach(&btn3, LONG_RRESS_START, BTN3_LONG_RRESS_START_Handler);
	button_attach(&btn3, LONG_PRESS_HOLD,  BTN3_LONG_PRESS_HOLD_Handler);
	button_attach(&btn3, LONG_PRESS_STOP,  BTN3_LONG_PRESS_STOP_Handler);
	button_start(&btn3);

	//make the timer invoking the button_ticks() interval 5ms.
	__timer_start(button_ticks, 0, 5); //This function is implemented by yourself.
	
	while(1) 
	{}
}