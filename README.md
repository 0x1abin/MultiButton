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

// 2. Define event callback (receives user_data)
void on_single_click(Button* btn, void* user_data)
{
    // handle single click
}

// 3. Initialize and start
void setup(void)
{
    button_init(&btn1, read_button_gpio, 0, 1);  // active low
    button_attach(&btn1, BTN_SINGLE_CLICK, on_single_click, NULL);
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
| `BTN_LONG_PRESS_HOLD` | Long press continuing (fires every tick while held) |

## State Machine

```
                          +-- long hold -->  [LONG_HOLD]
                          |                      |
[IDLE] -- press --> [PRESS]                   release
   ^                  |                          |
   |               release                       |
   |                  v                          |
   |             [RELEASE] <---------------------+
   |             |       ^
   |      timeout|       | quick press
   |             |       |
   +-------------+   [REPEAT] -- held too long --> [PRESS]
```

### State Transitions Detail

- **IDLE -> PRESS**: Button level matches active level after debounce. Fires `BTN_PRESS_DOWN`.
- **PRESS -> RELEASE**: Button released before long press threshold. Fires `BTN_PRESS_UP`.
- **PRESS -> LONG_HOLD**: Button held past `LONG_TICKS`. Fires `BTN_LONG_PRESS_START`.
- **RELEASE -> IDLE (timeout)**: No re-press within `SHORT_TICKS`. Fires `BTN_SINGLE_CLICK` or `BTN_DOUBLE_CLICK` based on repeat count.
- **RELEASE -> REPEAT**: Button pressed again within timeout. Fires `BTN_PRESS_DOWN` + `BTN_PRESS_REPEAT`.
- **REPEAT -> RELEASE**: Quick release. Continues waiting for more presses.
- **REPEAT -> PRESS**: Held too long in repeat state. Resets for a new press cycle.
- **LONG_HOLD -> IDLE**: Released from long press. Fires `BTN_PRESS_UP`.
- **LONG_HOLD (holding)**: Fires `BTN_LONG_PRESS_HOLD` every tick (see note below).

## API Reference

### Core Functions

```c
void button_init(Button* handle, uint8_t(*pin_level)(uint8_t),
                 uint8_t active_level, uint8_t button_id);
void button_attach(Button* handle, ButtonEvent event, BtnCallback cb, void* user_data);
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

### User Data (Context Pointer)

Every callback receives a `void* user_data` pointer, set via `button_attach()`:

```c
typedef struct {
    int led_pin;
    int beep_count;
} ButtonContext;

ButtonContext ctx = { .led_pin = 13, .beep_count = 0 };

void on_click(Button* btn, void* user_data)
{
    ButtonContext* ctx = (ButtonContext*)user_data;
    toggle_led(ctx->led_pin);
    ctx->beep_count++;
}

button_attach(&btn1, BTN_SINGLE_CLICK, on_click, &ctx);
```

All callbacks for the same button share the same `user_data` (it is stored per-button, not per-event).

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

Callbacks are executed **outside** the lock, so `button_stop()`/`button_start()` can be safely called from within callbacks without deadlock risk. A regular (non-recursive) mutex is sufficient.

## Implementing Triple Click (N-Click)

The library natively supports single click and double click events. For triple click or higher N-click, use the `BTN_PRESS_REPEAT` event combined with `button_get_repeat_count()`:

```c
void on_repeat_done(Button* btn, void* user_data)
{
    // This fires when repeat press is detected
    // Check repeat count after timeout for final count
}

void on_click_resolve(Button* btn, void* user_data)
{
    uint8_t count = button_get_repeat_count(btn);
    if (count == 3) {
        // Triple click!
    }
}

// Register for single click (fires after timeout with final repeat count)
button_attach(&btn, BTN_SINGLE_CLICK, on_click_resolve, NULL);
// Or check repeat count in any callback
button_attach(&btn, BTN_PRESS_REPEAT, on_repeat_done, NULL);
```

Note: `BTN_SINGLE_CLICK` fires when repeat==1 and `BTN_DOUBLE_CLICK` fires when repeat==2 after the short-press timeout. For repeat>=3, only `BTN_PRESS_REPEAT` fires during the press sequence. You can read `button_get_repeat_count()` from any callback to detect N-click patterns.

## Important Notes

### BTN_LONG_PRESS_HOLD fires every tick

`BTN_LONG_PRESS_HOLD` fires on **every tick** (default 5ms = 200Hz) while the button is held after the long press threshold. If your callback does expensive work, add your own throttling:

```c
void on_long_hold(Button* btn, void* user_data)
{
    static uint16_t throttle = 0;
    if (++throttle < 20) return;  // fire every 100ms instead
    throttle = 0;
    // ... do work ...
}
```

### Callback execution context

If `button_ticks()` is called from a timer interrupt (ISR), all callbacks execute in ISR context. Keep callbacks short and avoid blocking operations. For complex handling, set a flag in the callback and process it in the main loop.

If `button_ticks()` is called from a main loop or RTOS task, callbacks run in that context with no ISR restrictions.

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

## FAQ

**Q: How do I detect triple click?**
A: Register a `BTN_PRESS_REPEAT` callback and check `button_get_repeat_count()` for the desired count. See the "Implementing Triple Click" section above.

**Q: Is it safe to call `button_stop()` from inside a callback?**
A: Yes. The library caches the next-pointer before invoking callbacks, so removing a button during iteration is safe.

**Q: What happens if the ticks counter overflows during a very long press?**
A: The ticks counter saturates at `UINT16_MAX` (65535) instead of wrapping around. At 5ms intervals, this covers ~327 seconds of continuous holding.

**Q: Can I use this library in a multi-threaded RTOS?**
A: Yes. Define `MULTIBUTTON_THREAD_SAFE` and provide `MULTIBUTTON_LOCK()`/`MULTIBUTTON_UNLOCK()` macros. A regular (non-recursive) mutex is sufficient since callbacks execute outside the lock.

## Compatibility

- C99 standard
- Works on STM32, Arduino, ESP32, and other MCU platforms
- Supports bare-metal and RTOS environments
- Minimal memory footprint for resource-constrained systems

## Migration from v1.0/v1.1.0

v1.1.1 introduces breaking API changes:

1. **Callback signature changed**: Callbacks now receive `void* user_data` as a second parameter.
   ```c
   // v1.x: void callback(Button* btn)
   // v2.0: void callback(Button* btn, void* user_data)
   ```

2. **`button_attach()` has a new parameter**: Pass `NULL` if you don't need user data.
   ```c
   // v1.x: button_attach(&btn, BTN_SINGLE_CLICK, on_click);
   // v2.0: button_attach(&btn, BTN_SINGLE_CLICK, on_click, NULL);
   ```

## License

MIT License - see [LICENSE](LICENSE) for details.
