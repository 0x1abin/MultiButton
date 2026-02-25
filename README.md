# MultiButton

轻量级多按键状态机库，适用于各类嵌入式系统（STM32 / ESP32 / Arduino / bare-metal / RTOS）。

## 特性

- **4 状态有限状态机** — IDLE → PRESSED → RELEASED → LONG_HOLD，状态转换清晰可追踪
- **硬件消抖** — 可配置深度的数字滤波器（默认 3 × 5 ms = 15 ms）
- **多事件检测** — 按下、抬起、单击、双击、N 连击、长按开始、长按保持
- **双模式** — 回调模式 + 轮询模式，按需选用
- **用户上下文** — `user_data` 指针，便于面向对象风格的使用
- **编译期可配置** — `multi_button_cfg.h` 提供 `#ifndef` 守卫，支持零修改覆盖
- **零动态内存** — 按键实例由调用者静态分配，ISR 安全
- **单按键独立处理** — `button_tick_single()` 支持用户自行管理按键集合
- **双构建系统** — 同时提供 Makefile 与 CMakeLists.txt

## 目录结构

```
MultiButton/
├── include/
│   ├── multi_button.h          # 公共 API 头文件
│   └── multi_button_cfg.h      # 编译期配置（可覆盖）
├── src/
│   └── multi_button.c          # 状态机实现
├── examples/
│   ├── basic_example.c         # 基础示例
│   ├── advanced_example.c      # 高级示例（user_data / 动态回调）
│   └── poll_example.c          # 轮询模式示例
├── CMakeLists.txt              # CMake 构建
├── Makefile                    # GNU Make 构建
└── README.md
```

## 编译

### Make

```bash
make                # 编译库 + 全部示例
make library        # 仅编译静态库
make shared         # 编译动态库
make examples       # 仅编译示例
make test           # 编译并运行 basic_example
make clean          # 清理
make help           # 查看全部目标
```

### CMake

```bash
cmake -B build && cmake --build build
# 运行示例
./build/basic_example
```

### 自定义配置

在不修改库源码的情况下覆盖默认参数：

```bash
# Make
make EXTRA_CFLAGS='-DBTN_USER_CFG_FILE=\"my_cfg.h\"'

# CMake
cmake -B build -DBTN_USER_CFG_FILE='\"my_cfg.h\"'
```

或在编译命令中直接覆盖单个宏：

```bash
make EXTRA_CFLAGS='-DBTN_LONG_TICKS=400'
```

## 快速开始

### 1. 包含头文件

```c
#include "multi_button.h"
```

### 2. 实现 GPIO 读取函数

```c
uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id) {
    case 1: return HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin);
    default: return 0;
    }
}
```

### 3. 定义按键实例并初始化

```c
static button_t btn1;

button_init(&btn1, read_button_gpio, 0, 1);   // active_level=0 低电平有效
```

### 4. 注册事件回调

```c
void on_single_click(button_t *btn)
{
    const char *name = (const char *)button_get_user_data(btn);
    printf("%s: single click\n", name);
}

button_set_user_data(&btn1, "BTN1");
button_attach(&btn1, BTN_SINGLE_CLICK, on_single_click);
```

### 5. 启动 & 周期调用

```c
button_start(&btn1);

// 在 5 ms 定时器中断中：
void TIM_IRQHandler(void) {
    button_tick();
}
```

### 6. 轮询模式（可选）

```c
// 不注册回调，直接在主循环中查询
button_event_t ev = button_get_event(&btn1);
if (ev == BTN_SINGLE_CLICK) { /* ... */ }
```

## API 参考

### 事件类型 `button_event_t`

| 枚举值 | 说明 |
|---|---|
| `BTN_PRESS_DOWN` | 按键按下（消抖后） |
| `BTN_PRESS_UP` | 按键抬起 |
| `BTN_PRESS_REPEAT` | 连击序列中检测到重复按下 |
| `BTN_SINGLE_CLICK` | 单击完成（超时确认） |
| `BTN_DOUBLE_CLICK` | 双击完成（超时确认） |
| `BTN_LONG_PRESS_START` | 达到长按阈值 |
| `BTN_LONG_PRESS_HOLD` | 长按保持中（每个 tick 触发） |
| `BTN_NONE_PRESS` | 无事件 |

### 状态类型 `button_state_t`

| 枚举值 | 说明 |
|---|---|
| `BTN_STATE_IDLE` | 空闲 |
| `BTN_STATE_PRESSED` | 按下计时中 |
| `BTN_STATE_RELEASED` | 已释放，等待重按或超时 |
| `BTN_STATE_LONG_HOLD` | 长按保持 |

### 核心函数

| 函数 | 说明 |
|---|---|
| `button_init(btn, read_fn, active_level, id)` | 初始化按键实例 |
| `button_set_user_data(btn, ptr)` | 设置用户上下文指针 |
| `button_get_user_data(btn)` | 获取用户上下文指针 |
| `button_attach(btn, event, cb)` | 注册事件回调 |
| `button_detach(btn, event)` | 移除事件回调 |
| `button_start(btn)` | 加入全局 tick 链表（返回 0/-1/-2） |
| `button_stop(btn)` | 从全局 tick 链表移除 |
| `button_tick()` | 处理所有已注册按键（周期调用） |
| `button_tick_single(btn)` | 处理单个按键（自管理模式） |
| `button_get_event(btn)` | 获取当前事件（轮询用） |
| `button_get_state(btn)` | 获取 FSM 状态（调试用） |
| `button_get_repeat_count(btn)` | 获取连击计数 |
| `button_get_id(btn)` | 获取按键 ID |
| `button_is_pressed(btn)` | 查询当前是否处于按下状态 |
| `button_reset(btn)` | 重置为 IDLE 状态 |

## 配置选项

在 `include/multi_button_cfg.h` 中定义，均可通过 `#define` 在编译前覆盖：

| 宏 | 默认值 | 说明 |
|---|---|---|
| `BTN_TICKS_INTERVAL` | 5 | tick 周期 (ms) |
| `BTN_DEBOUNCE_TICKS` | 3 | 消抖深度 (max 7) |
| `BTN_SHORT_TICKS` | 60 | 短按 / 多击窗口 (ticks) |
| `BTN_LONG_TICKS` | 200 | 长按阈值 (ticks) |
| `BTN_REPEAT_MAX` | 15 | 最大连击计数 (max 15) |

## 状态机说明

```
                    ┌──────────┐
          press     │          │  release
    ┌───────────────►  PRESSED ├───────────────┐
    │               │          │               │
    │               └────┬─────┘               ▼
┌───┴────┐               │ held > LONG    ┌──────────┐
│  IDLE  │               │                │ RELEASED │
└───▲────┘               ▼                └──┬───┬───┘
    │          ┌───────────────┐   re-press  │   │
    │  release │   LONG_HOLD  │◄─────────────┘   │ timeout
    ├──────────┤  (hold events)│    ──► PRESSED   │
    │          └───────────────┘                  │
    │                                             │
    └──── single/double click ────────────────────┘
```

**4 状态，转换规则：**

| 当前状态 | 条件 | 动作 | 下一状态 |
|---|---|---|---|
| IDLE | 检测到按下 | 触发 PRESS_DOWN，repeat=1 | PRESSED |
| PRESSED | 检测到释放 | 触发 PRESS_UP | RELEASED |
| PRESSED | ticks > LONG | 触发 LONG_PRESS_START | LONG_HOLD |
| RELEASED | 检测到重按 | 触发 PRESS_DOWN + REPEAT，repeat++ | PRESSED |
| RELEASED | ticks > SHORT | 触发 SINGLE/DOUBLE_CLICK | IDLE |
| LONG_HOLD | 仍然按住 | 触发 LONG_PRESS_HOLD | LONG_HOLD |
| LONG_HOLD | 检测到释放 | 触发 PRESS_UP | IDLE |

## 与 v1.x 的差异

| 项目 | v1.x | v2.0 |
|---|---|---|
| 状态机 | 5 状态 (含 REPEAT) | 4 状态，消除歧义转换 |
| tick 计数器 | `uint16_t` (溢出风险) | `uint32_t` (安全) |
| 用户上下文 | 无 | `user_data` 指针 |
| 配置方式 | 修改头文件宏 | 独立 cfg.h + 编译期覆盖 |
| 单按键处理 | 仅全局 tick | 额外提供 `button_tick_single()` |
| 类型命名 | `Button` / `ButtonEvent` | `button_t` / `button_event_t` (C 风格) |
| 构建系统 | Makefile | Makefile + CMake |
| 文件结构 | 扁平 | `include/` + `src/` 分离 |
| const 正确性 | 无 | 查询函数接受 `const` 指针 |
| 版本信息 | 无 | `MULTI_BUTTON_VERSION_*` 宏 |

## 兼容性

- C99 标准
- 适用于各类微控制器 (STM32 / ESP32 / Arduino / NRF / CH32 等)
- 支持裸机 (bare-metal) 和 RTOS 环境
- 内存占用极小，适合资源受限系统

## 许可证

MIT License — 详见 [LICENSE](LICENSE) 文件。
