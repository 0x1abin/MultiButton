/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */
 
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stdint.h"
#include "string.h"

typedef void (*BtnCallback)(const struct Button* btn);

typedef enum {
	PRESS_DOWN = 0,
	PRESS_UP,
	PRESS_REPEAT,
	SINGLE_CLICK,
	DOUBLE_CLICK,
	LONG_RRESS_START,
	LONG_PRESS_HOLD,
	number_of_event，
	NONE_PRESS
}PressEvent;

struct Button {
	uint16_t ticks;
	uint8_t  repeat : 4;
	uint8_t  event : 4;
	uint8_t  state : 3;
	uint8_t  debounce_cnt : 3; 
	uint8_t  active_level : 1;
	uint8_t  button_level : 1;
	uint8_t  (*hal_button_Level)(void);
	BtnCallback  cb[number_of_event];
	struct Button* next;
};

#ifdef __cplusplus  
extern "C" {  
#endif  

void button_init(struct Button* handle, uint8_t(*pin_level)(), uint8_t active_level);
void button_attach(struct Button* handle, PressEvent event, CallBackFunc cb);
PressEvent get_button_event(const struct Button* handle);
int  button_start(struct Button* btn);
void button_stop(struct Button* btn);
void button_ticks(void);

#ifdef __cplusplus
} 
#endif  

#endif
