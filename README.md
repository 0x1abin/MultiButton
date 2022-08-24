# MultiButton

## 简介

MultiButton 是一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调异步处理方式可以简化你的程序结构，去除冗余的按键处理硬编码，让你的按键业务逻辑更清晰。

## 使用方法

1.先申请一个按键结构

```c
struct Button button1;
```

2.初始化按键对象，绑定按键的GPIO电平读取接口**read_button_pin()** ，后一个参数设置有效触发电平

```c
button_init(&button1, read_button_pin, 0);
```

3.注册按键事件

```c
button_attach(&button1, SINGLE_CLICK, BTN_EVENT_Handler);
button_attach(&button1, DOUBLE_CLICK, BTN_EVENT_Handler);
...
```

4.启动按键

```c
button_start(&button1);
```

5.设置一个5ms间隔的定时器循环调用后台处理函数

```c
while(1) {
    ...
    if(timer_ticks == 5) {
        timer_ticks = 0;

        button_ticks();
    }
}
```

## 特性

MultiButton 使用C语言实现，基于面向对象方式设计思路，每个按键对象单独用一份数据结构管理：

```c
struct Button {
	uint16_t ticks;
	uint8_t  repeat: 4;
	uint8_t  event : 4;
	uint8_t  state : 3;
	uint8_t  debounce_cnt : 3;
	uint8_t  active_level : 1;
	uint8_t  button_level : 1;
	uint8_t  (*hal_button_Level)(void);
	BtnCallback  cb[number_of_event];
	struct Button* next;
};
```

这样每个按键使用单向链表相连，依次进入 button_handler(struct Button* handle) 状态机处理，所以每个按键的状态彼此独立。

## 按键事件

| 事件             | 说明                                 |
| ---------------- | ------------------------------------ |
| PRESS_DOWN       | 按键按下，每次按下都触发             |
| PRESS_UP         | 按键弹起，每次松开都触发             |
| PRESS_REPEAT     | 重复按下触发，变量repeat计数连击次数 |
| SINGLE_CLICK     | 单击按键事件                         |
| DOUBLE_CLICK     | 双击按键事件                         |
| LONG_PRESS_START | 达到长按时间阈值时触发一次           |
| LONG_PRESS_HOLD  | 长按期间一直触发                     |

## Examples

```c
#include "multi_button.h"

typedef enum {
	BUTTON_ID_0,
	BUTTON_ID_2,

	BUTTON_ID_MAX

} BUTTON_ID_INDEX;

struct Button btnGroup[BUTTON_ID_MAX];
uint8_t btnActiveLevel[BUTTON_ID_MAX] = {0, 0};

uint8_t read_button_level()
{
	uint8_t code = 0x01;
	for (size_t i = 0; i < BUTTON_ID_MAX; i++)
	{
		if (get_button_current() == &btnGroup[i])
		{
			switch (i)
			{
			case BUTTON_ID_0:
				code = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
				break;
			case BUTTON_ID_2:
				code = HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin);
				break;

			default:
				break;
			}
		}
	}
	return code;
}

void BTN_EVENT_Handler(void *btn)
{
	for (size_t i = 0; i < BUTTON_ID_MAX; i++)
	{
		if (((Button *)btn) == &btnGroup[i])
		{
			switch (((Button *)btn)->event)
			{
			case PRESS_DOWN:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case PRESS_UP:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case PRESS_REPEAT:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case SINGLE_CLICK:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case DOUBLE_CLICK:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case LONG_PRESS_START:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;
			case LONG_PRESS_HOLD:
				switch (i)
				{
				case BUTTON_ID_0:
					// do something
					break;
				case BUTTON_ID_2:
					// do something
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}
	}
}

int main()
{
	for (size_t i = 0; i < BUTTON_ID_MAX; i++)
	{
		button_init(&btnGroup[i], read_button_level, btnActiveLevel[i]);

		button_attach(&btnGroup[i], PRESS_DOWN, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], PRESS_UP, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], PRESS_REPEAT, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], SINGLE_CLICK, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], DOUBLE_CLICK, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], LONG_PRESS_START, BTN_EVENT_Handler);
		button_attach(&btnGroup[i], LONG_PRESS_HOLD, BTN_EVENT_Handler);

		button_start(&btnGroup[i]);
	}

	// make the timer invoking the button_ticks() interval 5ms.
	// This function is implemented by yourself.
	__timer_start(button_ticks, 0, 5);

	while (1)
	{
	}
}
```
