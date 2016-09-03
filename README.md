# MultiButton

## 简介
MultiButton 是一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调处理方式可以简化你的程序逻辑，去除冗余的按键处理硬编码，专注你的程序思路。

## 使用方法
1.先申请一个按键结构

```
struct Button button1;
```
2.初始化按键对象，**read_button_pin()** 为按键的GPIO读取函数，后一个参数为设置触发电平

```
button_init(&button1, read_button_pin, 0);
```
3.注册按键事件

```
button_attach(&button1, CLICK,        Callback_CLICK_Handler);
button_attach(&button1, DOUBLE_CLICK, Callback_DOUBLE_Click_Handler);
...
```
4.启动按键

```
button_start(&button1);
```
5.设置一个5ms间隔的定时器循环调用后台处理函数

```
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

```
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
```
这样每个按键使用单向链表相连，依次进入 button_handler(struct Button* handle) 状态机处理，所以每个按键的状态彼此独立。


## Examples

```
#include "button.h"

struct Button button1;

int read_button_pin()
{
    return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);  //HAL GPIO read.
}

int main()
{
    button_init(&button1, read_button_pin, 0);
    button_attach(&button1, PRESSED,          BTN1_PRESSED_Handler);
    button_attach(&button1, CLICK,            BTN1_CLICK_Handler);
    button_attach(&button1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
    button_attach(&button1, LONG_RRESS_START, BTN1_LONG_RRESS_START_Handler);
    button_attach(&button1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);
    button_attach(&button1, LONG_PRESS_STOP,  BTN1_LONG_PRESS_STOP_Handler);
    button_start(&button1);
    
    //make the timer repeat invoking the button_ticks() interval 5ms.
    //This function is implemented by yourself.
    __timer_start(button_ticks, 0, 5);
    
    while(ture) 
    {
     ...
    }
}

void BTN1_SINGLE_CLICK_Handler()
{
    //do something..
}

void BTN1_DOUBLE_Click_Handler()
{
    //do something..
}
...
```

