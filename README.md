# MultiButton
Embedded event-driven button driver.


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
    button_init(&button1, read_K1_pin, 0);
    button_attach(&button1, SINGLE_CLICK,     BTN1_SINGLE_CLICK_Handler);
    button_attach(&button1, DOUBLE_CLICK,     BTN1_DOUBLE_Click_Handler);
    button_attach(&button1, LONG_RRESS_START, BTN1_LONG_RRESS_START_Handler);
    button_attach(&button1, LONG_PRESS_HOLD,  BTN1_LONG_PRESS_HOLD_Handler);
    button_attach(&button1, LONG_PRESS_STOP,  BTN1_LONG_PRESS_STOP_Handler);
    button_start(&button1);
    
    //make the timer repeat invoking the button_ticks() interval 5ms.
    __timer_start(button_ticks, 0, 5);
    
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

