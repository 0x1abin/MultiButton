# MultiButton

一个高效、灵活的多按键状态机库，支持多种按键事件检测。

## 功能特性

- **多种按键事件**: 按下、抬起、单击、双击、长按开始、长按保持、重复按下
- **硬件去抖**: 内置数字滤波，消除按键抖动
- **状态机驱动**: 清晰的状态转换逻辑，可靠性高
- **多按键支持**: 支持无限数量的按键实例
- **回调机制**: 灵活的事件回调函数注册，支持 `void* user_data` 上下文指针
- **内存优化**: 紧凑的数据结构，低内存占用
- **配置灵活**: 可自定义时间参数和功能选项
- **参数验证**: 完善的错误检查和边界条件处理
- **线程安全**: 可选的 RTOS 锁支持，裸机零开销

## 编译和构建

### 使用 Makefile (推荐)

```bash
# 编译所有内容 (库 + 示例)
make

# 只编译库
make library

# 只编译示例
make examples

# 编译特定示例
make basic_example
make advanced_example
make poll_example

# 运行测试
make test

# 清理构建文件
make clean

# 查看帮助
make help
```

### 构建输出

编译完成后，文件结构如下：

```
build/
+-- lib/
|   +-- libmultibutton.a    # 静态库
+-- bin/
|   +-- basic_example       # 基础示例
|   +-- advanced_example    # 高级示例
|   +-- poll_example        # 轮询示例
+-- obj/                    # 目标文件
```

## 快速开始

### 1. 包含头文件
```c
#include "multi_button.h"
```

### 2. 定义按键实例
```c
static Button btn1;
```

### 3. 实现 GPIO 读取函数
```c
uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id) {
        case 1:
            return HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin);
        default:
            return 0;
    }
}
```

### 4. 初始化按键
```c
// 初始化按键 (active_level: 0=低电平有效, 1=高电平有效)
button_init(&btn1, read_button_gpio, 0, 1);
```

### 5. 注册事件回调
```c
void btn1_single_click_handler(Button* btn, void* user_data)
{
    printf("Button 1: Single Click\n");
}

button_attach(&btn1, BTN_SINGLE_CLICK, btn1_single_click_handler, NULL);
```

### 6. 启动按键处理
```c
button_start(&btn1);
```

### 7. 定时调用处理函数
```c
// 在 5ms 定时器中断中调用
void timer_5ms_interrupt_handler(void)
{
    button_ticks();
}
```

## API 参考

### 按键事件类型
```c
typedef enum {
    BTN_PRESS_DOWN = 0,     // 按键按下
    BTN_PRESS_UP,           // 按键抬起
    BTN_PRESS_REPEAT,       // 重复按下检测
    BTN_SINGLE_CLICK,       // 单击完成
    BTN_DOUBLE_CLICK,       // 双击完成
    BTN_LONG_PRESS_START,   // 长按开始
    BTN_LONG_PRESS_HOLD,    // 长按保持 (每个 tick 触发)
    BTN_NONE_PRESS          // 无事件
} ButtonEvent;
```

### 核心函数

#### `void button_init(Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id)`
**功能**: 初始化按键实例
**参数**:
- `handle`: 按键句柄
- `pin_level`: GPIO 读取函数指针
- `active_level`: 有效电平 (0 或 1)
- `button_id`: 按键 ID

#### `void button_attach(Button* handle, ButtonEvent event, BtnCallback cb, void* user_data)`
**功能**: 注册事件回调函数
**参数**:
- `handle`: 按键句柄
- `event`: 事件类型
- `cb`: 回调函数
- `user_data`: 用户上下文指针（同一按键的所有回调共享）

#### `void button_detach(Button* handle, ButtonEvent event)`
**功能**: 移除事件回调函数

#### `int button_start(Button* handle)`
**功能**: 启动按键处理
**返回值**: 0=成功, -1=已存在, -2=参数错误

#### `void button_stop(Button* handle)`
**功能**: 停止按键处理（可在回调中安全调用）

#### `void button_ticks(void)`
**功能**: 后台处理函数 (每 5ms 调用一次)

### 工具函数

#### `ButtonEvent button_get_event(Button* handle)`
**功能**: 获取当前按键事件

#### `uint8_t button_get_repeat_count(Button* handle)`
**功能**: 获取重复按下次数

#### `void button_reset(Button* handle)`
**功能**: 重置按键状态

#### `int button_is_pressed(Button* handle)`
**功能**: 查询按键是否按下
**返回值**: 1=按下, 0=未按下, -1=错误

## user_data 上下文指针

每个回调函数都会收到一个 `void* user_data` 指针，通过 `button_attach()` 设置：

```c
typedef struct {
    int led_pin;
    int count;
} MyContext;

MyContext ctx = { .led_pin = 13, .count = 0 };

void on_click(Button* btn, void* user_data)
{
    MyContext* ctx = (MyContext*)user_data;
    toggle_led(ctx->led_pin);
    ctx->count++;
}

button_attach(&btn1, BTN_SINGLE_CLICK, on_click, &ctx);
```

同一按键的所有回调共享同一个 `user_data`（按键级别存储，非事件级别）。

## 实现三击 (N-Click)

库原生支持单击和双击事件。三击及以上可通过 `BTN_PRESS_REPEAT` + `button_get_repeat_count()` 实现：

```c
void on_repeat(Button* btn, void* user_data)
{
    uint8_t count = button_get_repeat_count(btn);
    if (count == 3) {
        // 三击!
    }
}

button_attach(&btn, BTN_PRESS_REPEAT, on_repeat, NULL);
```

说明: `BTN_SINGLE_CLICK` 在 repeat==1 时触发，`BTN_DOUBLE_CLICK` 在 repeat==2 时触发。repeat>=3 时，仅 `BTN_PRESS_REPEAT` 在按下过程中触发。

## 配置选项

在 `multi_button.h` 中可以自定义以下参数:

```c
#define TICKS_INTERVAL          5       // 定时器中断间隔 (ms)
#define DEBOUNCE_TICKS          3       // 去抖深度 (最大 7)
#define SHORT_TICKS             (300  / TICKS_INTERVAL)  // 短按阈值
#define LONG_TICKS              (1000 / TICKS_INTERVAL)  // 长按阈值
#define PRESS_REPEAT_MAX_NUM    15      // 最大重复计数
```

## 重要注意事项

### BTN_LONG_PRESS_HOLD 每 tick 触发

`BTN_LONG_PRESS_HOLD` 在长按保持期间 **每个 tick**（默认 5ms = 200Hz）触发一次。如果回调中有耗时操作，请自行节流：

```c
void on_long_hold(Button* btn, void* user_data)
{
    static uint16_t throttle = 0;
    if (++throttle < 20) return;  // 每 100ms 触发一次
    throttle = 0;
    // ... 执行操作 ...
}
```

### 回调执行上下文

如果 `button_ticks()` 在定时器中断（ISR）中调用，所有回调都在中断上下文中执行。回调应尽量简短，避免阻塞操作。建议在回调中设置标志位，在主循环中处理。

如果 `button_ticks()` 在主循环或 RTOS 任务中调用，回调在该上下文中执行，无 ISR 限制。

### 线程安全

```c
#define MULTIBUTTON_THREAD_SAFE
#define MULTIBUTTON_LOCK()   osMutexAcquire(btn_mutex, osWaitForever)
#define MULTIBUTTON_UNLOCK() osMutexRelease(btn_mutex)
#include "multi_button.h"
```

回调在锁**外**执行，因此可以在回调中安全调用 `button_stop()`/`button_start()`，不会死锁。使用普通互斥量即可（无需递归锁）。

## 状态机说明

```
   +------------- 抬起 --------- [LONG_HOLD]
   v                                     ^             
[IDLE] -- 按下 --> [PRESS] ----- 长按 ----+                                 
   ^                  |                      
   |               抬起                      
   |                  v                      
   |             [RELEASE]
   |             |       ^
   |          超时|       | 快速按下
   |             |       |
   +-------------+   [REPEAT] -- 按住太久 --> [PRESS]
```

## 示例程序

- `examples/basic_example.c` - 基础示例：单击、双击、长按检测
- `examples/advanced_example.c` - 高级示例：多按键管理、动态回调
- `examples/poll_example.c` - 轮询模式示例

## FAQ

**Q: 如何检测三击？**
A: 注册 `BTN_PRESS_REPEAT` 回调，在回调中用 `button_get_repeat_count()` 检查次数。

**Q: 在回调中调用 `button_stop()` 安全吗？**
A: 安全。库在调用回调前缓存了 next 指针，遍历过程中移除按键不会崩溃。

**Q: ticks 计数器会溢出吗？**
A: 不会。计数器在 `UINT16_MAX` (65535) 处饱和，不会回绕。在 5ms 间隔下可覆盖约 327 秒的持续按住。

**Q: 能在多线程 RTOS 中使用吗？**
A: 可以。定义 `MULTIBUTTON_THREAD_SAFE` 并提供锁宏。回调在锁外执行，普通互斥量即可。

## 兼容性

- C99 标准
- 适用于各种微控制器平台 (STM32, Arduino, ESP32, etc.)
- 支持裸机和 RTOS 环境
- 内存占用小，适合资源受限的系统

## 项目结构

```
MultiButton/
+-- multi_button.h          # 主头文件
+-- multi_button.c          # 主源文件
+-- Makefile               # 构建脚本
+-- examples/              # 示例目录
|   +-- basic_example.c    # 基础示例
|   +-- advanced_example.c # 高级示例
|   +-- poll_example.c     # 轮询示例
+-- tests/
|   +-- test_button.c      # 单元测试
+-- build/                 # 构建输出目录
+-- README.md              # 英文文档
+-- README_CN.md           # 中文文档
```
