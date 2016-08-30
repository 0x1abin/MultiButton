#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stdint.h"
#include "string.h"

typedef void (*CallBackFunc)(void);

typedef enum {
	SINGLE_CLICK = 0,
	DOUBLE_CLICK,
	LONG_RRESS_START,
	LONG_PRESS_HOLD,
	LONG_PRESS_STOP,
	number_of_event
}BtnEvent;

struct Button {
	uint16_t ticks;
	uint8_t  state : 3;
	uint8_t  debounce_cnt : 3; 
	uint8_t  active_level : 1;
	uint8_t  button_level : 1;
	uint8_t  (*hal_button_Level)(void);
	CallBackFunc  cb[number_of_event];
	struct Button* next;
};

#ifdef __cplusplus  
extern "C" {  
#endif  

void button_init(struct Button* handle, uint8_t(*pin_level)(), uint8_t active_level);
void button_attach(struct Button* handle, BtnEvent event, CallBackFunc cb);
int  button_is_pressed(struct Button* handle);
int  button_start(struct Button* btn);
void button_stop(struct Button* btn);
void button_ticks(void);

#ifdef __cplusplus
} 
#endif  

#endif
