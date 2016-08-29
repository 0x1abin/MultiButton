#include "button.h"
#include "string.h"

//MultiButton

#define HIGH							1
#define LOW								0
#define TICKS_INTERVAL		5	//ms

const uint8_t  kDebounceTicks  = 3;
const uint16_t kClickTicks     = (400/TICKS_INTERVAL);
const uint16_t kLongTicks      = (1000/TICKS_INTERVAL);

static struct Button* head_handle = NULL;


/**
  * @brief  Initializes the button struct handle.
  * @param  handle: the button handle strcut.
  * @param  pin_level: read the HAL GPIO of the connet button level.
  * @param  active_level: pressed GPIO level.
  * @retval None
  */
void button_init(struct Button* handle, uint8_t(*pin_level)(), uint8_t active_level)
{
	memset(handle, sizeof(struct Button), 0);
	handle->hal_button_Level = pin_level;
	handle->button_level = handle->hal_button_Level();
	handle->active_level = active_level;
}

/**
  * @brief  Attach the button event callback function.
  * @param  handle: the button handle strcut.
  * @param  event: trigger event type.
  * @param  cb: callback function.
  * @retval None
  */
void button_attach(struct Button* handle, BtnEvent event, CallBackFunc cb)
{
	handle->cb[event] = cb;
}

/**
  * @brief  Button driver core function, driver state machine.
  * @param  handle: the button handle strcut.
  * @retval None
  */
void button_handler(struct Button* handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level();

	//ticks counter working..
	if((handle->state) > 0) handle->ticks++;

	/*------------button debounce handle---------------*/
	if(read_gpio_level != handle->button_level) { //not equal to prev one
		//continue read 3 times same new level change
		if(++(handle->debounce_cnt) >= kDebounceTicks) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}

	} else { //leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if(handle->button_level == handle->active_level) {	//start press
			handle->ticks = 0;
			handle->state = 1;
		}
		break;

	case 1:
		if(handle->button_level != handle->active_level) { //released
			handle->state = 2;

		} else if(handle->ticks > kLongTicks) {
			if(handle->cb[LONG_RRESS_START]) handle->cb[LONG_RRESS_START]();

			handle->state = 5;
		}
		break;

	case 2:
		if(handle->ticks > kClickTicks) {	//released
			//press event
			if(handle->cb[CLICK]) handle->cb[CLICK]();	//signal click

			handle->state = 0;	//reset

		} else if(handle->button_level == handle->active_level) { //press again
			handle->state = 3;
		}
		break;

	case 3:	//repeat press pressing
		if(handle->button_level != handle->active_level) {	//double releasd
			//double click event
			if(handle->cb[DOUBLE_CLICK]) handle->cb[DOUBLE_CLICK]();

			handle->state = 0;
		}
		break;

	case 5:
		if(handle->button_level == handle->active_level) {
			//continue hold trigger
			if(handle->cb[LONG_PRESS_HOLD]) handle->cb[LONG_PRESS_HOLD]();

		} else { //releasd
			if(handle->cb[LONG_PRESS_STOP]) handle->cb[LONG_PRESS_STOP]();
			handle->state = 0; //reset
		}
		break;
	}
}

/**
  * @brief  Start the button work, add the handle into work list.
  * @param  btn: target handle strcut.
  * @retval None
  */
void button_start(struct Button* btn)
{
	btn->next = head_handle;
	head_handle = btn;
}

/**
  * @brief  Stop the button work, remove the handle off work list.
  * @param  btn: target handle strcut.
  * @retval None
  */
void button_stop(struct Button* btn)
{
	struct Button** curr;
	for(curr = &head_handle; *curr; ) {
		struct Button* entry = *curr;
		if (entry == btn) {
			*curr = entry->next;
//			free(entry);
		} else
			curr = &entry->next;
	}
}

/**
  * @brief  background ticks, timer repeat invoking interval 5ms.
  * @param  None.
  * @retval None
  */
void button_ticks()
{
	struct Button* target;
	for(target=head_handle; target; target=target->next) {
		button_handler(target);
	}
}



/*

struct Button btn1;
struct Button btn2;
struct Button btn3;

void main()
{
	button_init(&btn1, read_K1_pin, 0);
	button_attach(&btn1, CLICK,            BTN1_Click_Handler);
	button_attach(&btn1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
	button_attach(&btn1, LONG_RRESS_START, BTN1_LONG_RRESS_START_Handler);
	button_attach(&btn1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);
	button_attach(&btn1, LONG_PRESS_STOP,  BTN1_LONG_PRESS_STOP_Handler);
	button_start(&btn1);
	
	button_init(&btn2, read_K2_pin, 0);
	button_attach(&btn2, CLICK,            BTN2_Click_Handler);
	button_attach(&btn2, DOUBLE_CLICK,     BTN2_DOUBLE_Click_Handler);
	button_attach(&btn2, LONG_RRESS_START, BTN2_LONG_RRESS_START_Handler);
	button_attach(&btn2, LONG_PRESS_HOLD,  BTN2_LONG_PRESS_HOLD_Handler);
	button_attach(&btn2, LONG_PRESS_STOP,  BTN2_LONG_PRESS_STOP_Handler);
	button_start(&btn2);
	
	button_init(&btn3, read_K3_pin, 0);
	button_attach(&btn3, CLICK,            BTN3_Click_Handler);
	button_attach(&btn3, DOUBLE_CLICK,     BTN3_DOUBLE_Click_Handler);
	button_attach(&btn3, LONG_RRESS_START, BTN3_LONG_RRESS_START_Handler);
	button_attach(&btn3, LONG_PRESS_HOLD,  BTN3_LONG_PRESS_HOLD_Handler);
	button_attach(&btn3, LONG_PRESS_STOP,  BTN3_LONG_PRESS_STOP_Handler);
	button_start(&btn3);

	//make the timer invoking the button_ticks() interval 5ms.
	// timer_start(button_ticks, 0, 5);	
	
	while(1)
	{

	}

	button_stop(&btn1);
	button_stop(&btn2);
	button_stop(&btn3);
}

*/






















