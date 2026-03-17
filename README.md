# MultiButton

A compact and flexible multi-button state machine library for embedded systems.

[中文文档 (Chinese)](README_CN.md)

## Features

- **7 event types**: press down, press up, single click, double click, long press start, long press hold, repeat press
- **Hardware debounce**: built-in digital filter eliminates contact bounce
- **State machine driven**: reliable state transitions with clear logic
- **Unlimited buttons**: linked-list architecture supports any number of button instances
- **Callback & polling**: flexible event handling via callbacks or polling `button_get_event()`
- **Memory efficient**: compact bitfield struct (~30 bytes per button)
- **Configurable**: adjustable timing thresholds and debounce depth
- **Thread-safe option**: optional RTOS lock hooks with zero overhead on bare-metal

## Quick Start

```c
#include "multi_button.h"

static Button btn1;

// 1. Implement GPIO read function
uint8_t read_button_gpio(uint8_t button_id)
{
    return HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin);
}

// 2. Define event callback
void on_single_click(Button* btn)
{
    // handle single click
}

// 3. Initialize and start
void setup(void)
{
    button_init(&btn1, read_button_gpio, 0, 1);  // active low
    button_attach(&btn1, BTN_SINGLE_CLICK, on_single_click);
    button_start(&btn1);
}

// 4. Call from 5ms timer interrupt
void timer_5ms_isr(void)
{
    button_ticks();
}
```

## Event Types

| Event | Description |
|-------|-------------|
| `BTN_PRESS_DOWN` | Button pressed down |
| `BTN_PRESS_UP` | Button released |
| `BTN_PRESS_REPEAT` | Repeated press detected |
| `BTN_SINGLE_CLICK` | Single click completed (after timeout) |
| `BTN_DOUBLE_CLICK` | Double click completed (after timeout) |
| `BTN_LONG_PRESS_START` | Long press threshold reached (fires once) |
| `BTN_LONG_PRESS_HOLD` | Long press continuing (fires repeatedly) |

## State Machine

```
[IDLE] --press--> [PRESS] --long hold--> [LONG_HOLD]
   ^                 |                       |
   |            release|                  release|
   |                 v                       |
   |            [RELEASE] <------------------+
   |            |       ^
   |     timeout|       |quick press
   |            |       |
   +------------+   [REPEAT]
```

## API Reference

### Core Functions

```c
void button_init(Button* handle, uint8_t(*pin_level)(uint8_t),
                 uint8_t active_level, uint8_t button_id);
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb);
void button_detach(Button* handle, ButtonEvent event);
int  button_start(Button* handle);   // returns 0=ok, -1=duplicate, -2=invalid
void button_stop(Button* handle);
void button_ticks(void);             // call every 5ms from timer
```

### Utility Functions

```c
ButtonEvent button_get_event(Button* handle);        // current event (polling mode)
uint8_t     button_get_repeat_count(Button* handle);  // repeat press count
int         button_is_pressed(Button* handle);        // 1=pressed, 0=released, -1=error
void        button_reset(Button* handle);             // reset to idle state
```

## Configuration

Edit the defines in `multi_button.h`:

```c
#define TICKS_INTERVAL       5     // timer tick interval (ms)
#define DEBOUNCE_TICKS       3     // debounce filter depth (max 7)
#define SHORT_TICKS          (300  / TICKS_INTERVAL)  // short press threshold
#define LONG_TICKS           (1000 / TICKS_INTERVAL)  // long press threshold
#define PRESS_REPEAT_MAX_NUM 15    // max repeat counter
```

## Thread Safety (RTOS)

For RTOS environments, define lock macros before including the header:

```c
#define MULTIBUTTON_THREAD_SAFE
#define MULTIBUTTON_LOCK()   osMutexAcquire(btn_mutex, osWaitForever)
#define MULTIBUTTON_UNLOCK() osMutexRelease(btn_mutex)
#include "multi_button.h"
```

On bare-metal (default), the lock macros compile to nothing with zero overhead.

## Building

```bash
# Make
make all          # library + examples
make test         # run unit tests
make library      # static library only

# CMake
cmake -B build -DMULTIBUTTON_BUILD_TESTS=ON -DMULTIBUTTON_BUILD_EXAMPLES=ON
cmake --build build
cd build && ctest
```

## Examples

- `examples/basic_example.c` - Single/double click, long press, repeat detection
- `examples/advanced_example.c` - Multi-button management, dynamic callback attach/detach
- `examples/poll_example.c` - Polling mode without callbacks

## Compatibility

- C99 standard
- Works on STM32, Arduino, ESP32, and other MCU platforms
- Supports bare-metal and RTOS environments
- Minimal memory footprint for resource-constrained systems

## License

MIT License - see [LICENSE](LICENSE) for details.
