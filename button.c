#include "button.h"

//MultiButton

#define HIGH              1
#define LOW               0
#define TICKS_INTERVAL    5	//ms

//According to your need to modify the constants.
const uint8_t  kDebounceTicks  = 3;	//MAX 3
const uint16_t kClickTicks     = (400/TICKS_INTERVAL);
const uint16_t kLongTicks      = (1000/TICKS_INTERVAL);

//button handle list head.
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
			if(handle->cb[SINGLE_CLICK]) handle->cb[SINGLE_CLICK]();	//signal click

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
  * @retval 0: succeed. -1: already exist.
  */
int button_start(struct Button* btn)
{
	struct Button* target = head_handle;
	while(target) {
		if(target == btn) return -1;	//already exist.
		target = target->next;
	}
	btn->next = head_handle;
	head_handle = btn;
	return 0;
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

