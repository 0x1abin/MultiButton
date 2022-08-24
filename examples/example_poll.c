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

int main()
{
	static uint8_t btn_event_val[BUTTON_ID_MAX] = {NONE_PRESS, NONE_PRESS};

	for (size_t i = 0; i < BUTTON_ID_MAX; i++)
	{
		button_init(&btnGroup[i], read_button_level, btnActiveLevel[i]);

		button_start(&btnGroup[i]);
	}

	// make the timer invoking the button_ticks() interval 5ms.
	// This function is implemented by yourself.
	__timer_start(button_ticks, 0, 5);

	while (1)
	{
		for (size_t i = 0; i < BUTTON_ID_MAX; i++)
		{
			if (btn_event_val[i] != get_button_event(&btnGroup[i]))
			{
				btn_event_val[i] = get_button_event(&btnGroup[i]);

				if (btn_event_val[i] == PRESS_DOWN)
				{
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
				}
				else if (btn_event_val[i] == PRESS_UP)
				{
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
				}
				else if (btn_event_val[i] == LONG_PRESS_HOLD)
				{
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
				}
			}
		}
	}
}
