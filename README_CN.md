# MultiButton

一个高效、灵活的多按键状态机库，支持多种按键事件检测。

## 功能特性

- ✅ **多种按键事件**: 按下、抬起、单击、双击、长按开始、长按保持、重复按下
- ✅ **硬件去抖**: 内置数字滤波，消除按键抖动
- ✅ **状态机驱动**: 清晰的状态转换逻辑，可靠性高
- ✅ **多按键支持**: 支持无限数量的按键实例
- ✅ **回调机制**: 灵活的事件回调函数注册
- ✅ **内存优化**: 紧凑的数据结构，低内存占用
- ✅ **配置灵活**: 可自定义时间参数和功能选项
- ✅ **参数验证**: 完善的错误检查和边界条件处理

## 优化改进

### 1. 代码结构优化
- 更清晰的枚举命名 (`BTN_PRESS_DOWN` vs `PRESS_DOWN`)
- 增加状态机状态枚举，提高可读性
- 统一的函数命名规范
- 更好的代码注释和文档

### 2. 功能增强
- 新增 `button_detach()` - 动态移除事件回调
- 新增 `button_reset()` - 重置按键状态
- 新增 `button_is_pressed()` - 查询当前按键状态
- 新增 `button_get_repeat_count()` - 获取重复按下次数
- 改进的 `button_get_event()` 函数

### 3. 安全性提升
- 完善的参数验证
- 空指针检查
- 数组越界保护
- 更好的错误返回值

### 4. 性能优化
- 内联函数优化 GPIO 读取
- 更安全的宏定义
- 减少不必要的计算
- 优化的状态机逻辑

### 5. 可维护性
- 清晰的状态转换
- 模块化设计
- 配置文件分离
- 详细的使用示例

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

### 使用构建脚本

```bash
# 使脚本可执行
chmod +x build.sh

# 编译所有内容
./build.sh

# 只编译库
./build.sh library

# 编译特定示例
./build.sh basic_example

# 查看帮助
./build.sh help
```

### 构建输出

编译完成后，文件结构如下：

```
build/
├── lib/
│   └── libmultibutton.a    # 静态库
├── bin/
│   ├── basic_example       # 基础示例
│   ├── advanced_example    # 高级示例
│   └── poll_example        # 轮询示例
└── obj/                    # 目标文件
```

## 示例程序

### 1. 基础示例 (`examples/basic_example.c`)

演示基本的按键事件处理：

```bash
./build/bin/basic_example
```

功能：
- 单击、双击、长按检测
- 重复按下计数
- 按键状态查询
- 自动化演示序列

### 2. 高级示例 (`examples/advanced_example.c`)

演示高级功能和动态管理：

```bash
# 运行完整演示
./build/bin/advanced_example

# 详细输出模式
./build/bin/advanced_example -v

# 安静模式 (手动测试)
./build/bin/advanced_example -q
```

功能：
- 多按键管理
- 动态回调函数添加/移除
- 配置按键
- 运行时状态监控

### 3. 轮询示例 (`examples/poll_example.c`)

演示轮询模式使用：

```bash
./build/bin/poll_example
```

功能：
- 无回调函数的轮询模式
- 事件状态查询
- 主循环集成示例
- 预定义按键模式演示

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
void btn1_single_click_handler(void* btn)
{
    printf("Button 1: Single Click\n");
}

button_attach(&btn1, BTN_SINGLE_CLICK, btn1_single_click_handler);
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
    BTN_LONG_PRESS_HOLD,    // 长按保持
    BTN_NONE_PRESS          // 无事件
} ButtonEvent;
```

### 核心函数

#### `void button_init(Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id)`
**功能**: Initialize button instance  
**参数**: 
- `handle`: 按键句柄
- `pin_level`: GPIO 读取函数指针
- `active_level`: 有效电平 (0 或 1)
- `button_id`: 按键 ID

#### `void button_attach(Button* handle, ButtonEvent event, BtnCallback cb)`
**功能**: Attach event callback function  
**参数**:
- `handle`: 按键句柄
- `event`: 事件类型
- `cb`: 回调函数

#### `void button_detach(Button* handle, ButtonEvent event)`
**功能**: Detach event callback function  
**参数**:
- `handle`: 按键句柄  
- `event`: 事件类型

#### `int button_start(Button* handle)`
**功能**: Start button processing  
**返回值**: 0=成功, -1=已存在, -2=参数错误

#### `void button_stop(Button* handle)`
**功能**: Stop button processing

#### `void button_ticks(void)`
**功能**: Background processing function (call every 5ms)

### 工具函数

#### `ButtonEvent button_get_event(Button* handle)`
**功能**: Get current button event

#### `uint8_t button_get_repeat_count(Button* handle)`
**功能**: Get repeat press count

#### `void button_reset(Button* handle)`
**功能**: Reset button state to idle

#### `int button_is_pressed(Button* handle)`
**功能**: Check if button is currently pressed  
**返回值**: 1=按下, 0=未按下, -1=错误

## 配置选项

在 `multi_button_config.h` 中可以自定义以下参数:

```c
#define TICKS_INTERVAL          5       // 定时器中断间隔 (ms)
#define DEBOUNCE_TIME_MS        15      // 去抖时间 (ms)
#define SHORT_PRESS_TIME_MS     300     // 短按时间阈值 (ms)
#define LONG_PRESS_TIME_MS      1000    // 长按时间阈值 (ms)
#define PRESS_REPEAT_MAX_NUM    15      // 最大重复计数
```

## 使用注意事项

1. **定时器设置**: 必须配置 5ms 定时器中断，在中断中调用 `button_ticks()`
2. **GPIO 配置**: 按键引脚需配置为输入模式，根据需要启用上拉或下拉电阻
3. **回调函数**: 回调函数应尽量简短，避免长时间阻塞
4. **内存管理**: 按键实例可以是全局变量或动态分配
5. **多按键**: 每个物理按键需要独立的 Button 实例和唯一的 button_id

## 状态机说明

```
[IDLE] --按下--> [PRESS] --长按--> [LONG_HOLD]
   ^                |                    |
   |             抬起|                 抬起|
   |                v                    |
   |          [RELEASE] <----------------+
   |          |       ^
   |       超时|       |快速按下
   |          |       |
   +----------+   [REPEAT]
```

## 项目结构

```
MultiButton/
├── multi_button.h          # 主头文件
├── multi_button.c          # 主源文件
├── Makefile               # 构建脚本
├── build.sh               # 备用构建脚本
├── examples/              # 示例目录
│   ├── basic_example.c    # 基础示例
│   ├── advanced_example.c # 高级示例
│   └── poll_example.c     # 轮询示例
├── build/                 # 构建输出目录
│   ├── lib/              # 库文件
│   ├── bin/              # 可执行文件
│   └── obj/              # 目标文件
└── README.md             # 说明文档
```

## 兼容性

- C99 标准
- 适用于各种微控制器平台 (STM32, Arduino, ESP32, etc.)
- 支持裸机和 RTOS 环境
- 内存占用小，适合资源受限的系统
