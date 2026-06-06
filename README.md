# EmbeddedKit (ek_utils)

面向资源受限 MCU 环境的轻量级嵌入式 C 工具库。提供数据结构、内存管理、日志、事件调度等裸机和 RTOS 开发的通用基础设施。

## 特性

- **零依赖**：无需 RTOS，可直接集成到裸机项目
- **配置驱动**：所有模块通过 `ek_conf.h` 宏开关按需裁剪，编译期零开销
- **跨编译器**：支持 GCC、ARM Compiler 5 (armcc)、ARM Compiler 6 (armclang)
- **弱函数扩展**：平台相关接口通过 `__EK_WEAK` 弱符号声明，用户提供强函数覆盖

## 项目结构

```
ek_utils/
├── cmake/                         # 交叉编译工具链
│   └── gcc-arm-none-eabi.cmake
├── CMakeLists.txt                 # 构建入口（对象库，子模块友好）
├── ek_conf_template.h             # 配置模板 → 复制到项目改名为 ek_conf.h
├── inc/                           # 头文件
│   ├── ek_conf_internal.h         # 内部配置入口（引入用户 ek_conf.h + 默认值 + 校验）
│   ├── ek_def.h                   # 跨编译器兼容层（__EK_WEAK / __EK_PACKED 等）
│   ├── ek_err.h                   # 错误码定义（ek_err_t）和错误处理宏
│   ├── ek_assert.h                # 断言模块
│   ├── ek_log.h                   # 分级日志模块
│   ├── ek_io.h                    # IO 输出封装（lwprintf / picolibc / 空实现）
│   ├── ek_heap.h                  # 内存堆管理（基于 TLSF）
│   ├── ek_list.h                  # 双向循环链表（纯头文件）
│   ├── ek_vec.h                   # 类型安全动态数组（纯头文件，宏生成类型）
│   ├── ek_ringbuf.h               # 环形缓冲区（通用 + SPSC 无锁变体）
│   ├── ek_stack.h                 # 通用 LIFO 栈
│   ├── ek_str.h                   # 动态字符串
│   ├── ek_export.h                # 函数自动导出初始化（类似 Linux initcall）
│   ├── ek_evoke.h                 # 协作式事件驱动任务调度器
│   └── ek_picothread.h            # 微线程 + 信号量 + 消息队列
├── src/                           # 源文件
│   ├── ek_assert.c
│   ├── ek_evoke.c
│   ├── ek_export.c
│   ├── ek_heap.c
│   ├── ek_io.c
│   ├── ek_log.c
│   ├── ek_picothread.c
│   ├── ek_picolibc_port.c          # picolibc 适配层
│   ├── ek_ringbuf.c
│   ├── ek_stack.c
│   └── ek_str.c
├── third_party/                   # 第三方代码
│   ├── tlsf/                       # TLSF 实时内存分配器（O(1) 分配/释放）
│   ├── lwprintf/                   # 轻量级 printf 实现
│   └── picolibc/                   # 轻量级 C 标准库（含 Cortex-M 预编译库）
└── README.md
```

## 模块清单

| 模块 | 配置宏 | 类型 | 说明 |
|------|--------|------|------|
| `ek_err` | 始终包含 | 基础层 | 错误码体系（`ek_err_t`），含 45+ 错误码和错误传播宏 |
| `ek_def` | 始终包含 | 基础层 | 跨编译器兼容层（`__EK_WEAK`/`__EK_PACKED`/`__EK_INLINE` 等） |
| `ek_io` | `EKCFG_IO_LWPRTF` / `EKCFG_PICOLIBC` | 核心服务 | IO 输出封装，支持 lwprintf 或 picolibc 两种后端 |
| `ek_log` | `EKCFG_LOG` | 核心服务 | 分级日志（DEBUG/INFO/WARN/ERROR），支持 ANSI 彩色和时间戳 |
| `ek_assert` | `EKCFG_ASSERT` | 核心服务 | 断言：tiny 模式（死循环）和 full 模式（输出详情） |
| `ek_heap` | `EKCFG_HEAP_TLSF` | 核心服务 | 基于 TLSF 的内存堆管理，支持多内存池 |
| `ek_list` | `EKCFG_LIST` | 数据结构 | Linux 内核风格双向循环链表（纯头文件，侵入式设计） |
| `ek_vec` | `EKCFG_VEC` | 数据结构 | 类型安全动态数组（纯头文件，宏生成类型，依赖 `ek_heap`） |
| `ek_ringbuf` | `EKCFG_RINGBUF` | 数据结构 | 通用环形缓冲区（RTOS 模式下含锁） |
| `ek_ringbuf_spsc` | `EKCFG_RINGBUF_SPSC` | 数据结构 | SPSC 单生产者单消费者无锁环形缓冲区 |
| `ek_stack` | `EKCFG_STACK` | 数据结构 | 通用 LIFO 栈 |
| `ek_str` | `EKCFG_STR` | 数据结构 | 自动扩容的动态字符串 |
| `ek_export` | `EKCFG_EXPORT` | 系统服务 | 基于链接器段的自动初始化（需配合链接脚本） |
| `ek_evoke` | `EKCFG_EVOKE` | 系统服务 | 协作式事件驱动调度器（仅裸机，`EKCFG_RTOS=0` 时可用） |
| `ek_picothread` | `EKCFG_PICOTHREAD` | 系统服务 | 协作式微线程调度器（protothread，优先级就绪/阻塞链表） |
| `ek_picothread_sem` | `EKCFG_PICOTHREAD_SEM` | 系统服务 | 微线程信号量（依赖 `ek_picothread`） |
| `ek_picothread_msg` | `EKCFG_PICOTHREAD_MSG` | 系统服务 | 微线程消息队列（依赖 `ek_picothread` + `ek_ringbuf`） |

## 快速上手

### 支持平台

| PLATFORM | CPU | FPU |
|----------|-----|-----|
| cm0 | Cortex-M0 | 无 |
| cm3 | Cortex-M3 | 无 |
| cm4 | Cortex-M4 | fpv4-sp-d16 (hard) |
| cm7 | Cortex-M7 | fpv5-d16 (hard) |
| cm23 | Cortex-M23 | 无 |
| cm33 | Cortex-M33 | fpv5-sp-d16 (hard) |
| cm55 | Cortex-M55 | fpv5-auto (hard) |

### 方式一：子模块集成（推荐）

```bash
# 在你的 MCU 项目根目录
git submodule add https://github.com/N1netyNine99/ek_utils.git lib/ek_utils
```

CMakeLists.txt 配置：

```cmake
# 1. 你的 include 路径在前（确保 ek_conf.h 优先被找到）
target_include_directories(${PROJECT_NAME} PRIVATE Core/Inc)

# 2. 引入 ek_utils 对象库
add_subdirectory(lib/ek_utils)
target_link_libraries(${PROJECT_NAME} $<TARGET_OBJECTS:ek_utils>)

# 3. 可选：通过 EK_CONF_PATH 显式指定 ek_conf.h 位置
#    cmake -DEK_CONF_PATH="Core/Inc/ek_conf.h" ...
```

### 方式二：源文件直接集成

```
Include Paths:
  - inc/
  - third_party/tlsf/
  - third_party/lwprintf/inc/

源文件（按需选择）:
  - src/*.c（自动收集所有 .c 文件）

第三方源文件（始终编译，由 #if 守卫控制实际使用）:
  - third_party/tlsf/tlsf.c
  - third_party/lwprintf/lwprintf.c
```

## 配置说明

ek_utils 不内置默认配置文件。**用户必须创建自己的 `ek_conf.h`**，否则编译报错。

### 创建配置文件

1. 复制仓库根目录的 `ek_conf_template.h` → 你的项目 include 路径，改名为 `ek_conf.h`
2. 按硬件调整宏值（不需要改的可以删掉，未定义的宏自动取默认值）
3. 确保编译器能找到你的 `ek_conf.h`

### 指定配置文件的两种方式

**方式 A：CMake 传参（推荐）**

```bash
cmake -DEK_CONF_PATH="Core/Inc/ek_conf.h" -B build
```

CMakeLists.txt 中的 `target_compile_definitions` 会自动把路径传给编译器，
`ek_conf_internal.h` 会 `#include` 指定路径。

**方式 B：include 优先级**

确保你的 `ek_conf.h` 所在目录在 include 搜索路径中优先于 `ek_utils/inc/`。
在 CMake 中，`target_include_directories` 写在 `add_subdirectory` 之前的路径优先级更高。

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE Core/Inc)  # 在前 → 优先
add_subdirectory(lib/ek_utils)
```

### 配置宏完整列表

```c
// 平台/运行环境
#define EKCFG_RTOS      (0)     // 是否使用 RTOS
#define EKCFG_PICOLIBC  (1)     // 是否使用 picolibc（自动关闭 lwprintf）
#define EKCFG_IO_LWPRTF (0)     // IO 是否使用 lwprintf

// 核心服务（1=启用，0=禁用）
#define EKCFG_EXPORT (0)
#define EKCFG_ASSERT (1)
#define EKCFG_LOG    (1)

// 数据结构（1=启用，0=禁用）
#define EKCFG_STR            (0)
#define EKCFG_LIST           (0)
#define EKCFG_VEC            (0)
#define EKCFG_RINGBUF        (0)
#define EKCFG_RINGBUF_SPSC   (0)
#define EKCFG_STACK          (0)

// 系统服务（1=启用，0=禁用）
#define EKCFG_EVOKE          (0)   // 仅裸机 (EKCFG_RTOS=0 时可用)
#define EKCFG_PICOTHREAD     (0)   // 协作式微线程调度器
#define EKCFG_PICOTHREAD_SEM (0)   // 微线程信号量 (依赖 PICOTHREAD)
#define EKCFG_PICOTHREAD_MSG (0)   // 微线程消息队列 (依赖 PICOTHREAD)

// 子配置
#define EKCFG_HEAP_TLSF                (1)           // 使用 TLSF 分配器
#define EKCFG_HEAP_SIZE                (30 * 1024)   // 堆大小（按 MCU SRAM 调整）
#define EKCFG_HEAP_SECTION             ".tcmram"     // 堆所在链接器段
#define EKCFG_LOG_DEBUG                (1)           // 启用 DEBUG 日志
#define EKCFG_LOG_COLOR                (1)           // ANSI 彩色日志
#define EKCFG_LOG_BUF_SIZE             (256)         // 日志缓冲区
#define EKCFG_ASSERT_TINY              (1)           // 轻量级断言
#define EKCFG_ASSERT_LOG               (1)           // 断言时输出日志
#define EKCFG_EVOKE_MIN_DEEPSLEEP_TICK (10)          // 进入深度睡眠最小 tick
```

### picolibc vs lwprintf

| 场景 | 配置 | 说明 |
|------|------|------|
| 标准 IO (picolibc) | `EKCFG_PICOLIBC=1` | ek_printf → printf，ek_malloc → picolibc malloc |
| 轻量 IO (lwprintf) | `EKCFG_IO_LWPRTF=1, EKCFG_PICOLIBC=0` | ek_printf → lwprintf，适合没有完整 libc 的环境 |

### 原理解释

```
库源文件
  └→ #include "ek_conf_internal.h"
       ├→ #include 用户的 ek_conf.h     （用户定义的宏）
       ├→ 填充用户未定义的宏（#ifndef 守卫）
       └→ 依赖校验（picolibc 冲突、evoke+RTOS 冲突等）
```

用户只需定义要改的宏，其余自动取默认值。

## 平台适配

### 弱函数（Weak Functions）

所有需要平台或用户定制的函数以 `__EK_WEAK` 声明，用户提供同名强函数即可覆盖：

```c
// ========== ek_io — 字符输出 ==========
void _ek_io_fputc(int ch)               // 字符输出（UART 等）

// ========== ek_log — 时间戳 ==========
uint32_t _ek_log_get_tick(void)          // 系统时间戳

// ========== ek_evoke — 临界区 & 睡眠（共 5 个）==========
void ek_evoke_enter_critical(void)       // 进入临界区（关中断）
void ek_evoke_exit_critical(void)        // 退出临界区（开中断）
void ek_evoke_set_timer(uint32_t xtick)  // 设置单次定时器
void ek_evoke_light_sleep(void)          // 浅睡眠（WFI）
void ek_evoke_deep_sleep(void)           // 深度睡眠（低功耗模式）

// ========== ek_heap — 自定义分配器（仅在 EKCFG_HEAP_TLSF=0 时）==========
void *ek_malloc(size_t size)
void ek_free(void *ptr)
void *ek_realloc(void *ptr, size_t size)
```

### 在 STM32 项目中的典型实现

```c
// ek_port.c
#include "ek_io.h"
#include "ek_log.h"

void _ek_io_fputc(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
}

uint32_t _ek_log_get_tick(void)
{
    return HAL_GetTick();
}
```

## 设计约定

### 错误码体系

所有非 `void` 返回的操作函数使用 `ek_err_t`（`uint8_t`）返回错误码：

```c
ek_err_t ek_ringbuf_write(ek_ringbuf_t *rb, const void *item);
// 返回 EK_ERR_NONE(0) 成功，EK_ERR_FULL(207) 满，EK_ERR_BUSY(202) 忙

// 错误传播宏
EK_ERR_RETURN(ek_str_append(s, "hello"));  // 非零则直接 return
EK_ERR_GOTO(ek_foo(), cleanup);             // 非零则 goto cleanup
```

`bool` 返回值仅用于纯查询函数（`ek_ringbuf_full`、`ek_stack_empty` 等）。

### _safely 释放宏

释放后将指针置 NULL，防止悬空指针重用：

```c
ek_free_safely(ptr);                    // ptr 释放后自动 = NULL
ek_ringbuf_destroy_safely(rb);          // rb 释放后自动 = NULL
ek_stack_destroy_safely(sk);            // sk 释放后自动 = NULL
```

### 命名规范

- 公共 API：`ek_` 前缀（`ek_malloc`、`ek_ringbuf_create`）
- 全局变量：`g_` 前缀（`g_ek_default_heap`）
- 静态变量：`s_` 前缀（`s_lock`、`s_log_buffer`）
- 内部静态函数：`_` 前缀 + 文件顶部前向声明 + 文件底部实现
- 配置宏：`EKCFG_` 前缀，正逻辑命名（1=启用，0=禁用）

### 侵入式数据结构

`ek_list` 和 `ek_evoke` 使用侵入式链表节点（将 `ek_list_node_t` 嵌入用户结构体），避免额外内存分配。

## 第三方依赖

| 库 | 用途 | 许可证 |
|----|------|--------|
| [TLSF](http://tlsf.baisoku.org) v3.1 | 实时内存分配器（O(1) 分配/释放） | BSD-style |
| [lwprintf](https://github.com/MaJerle/lwprintf) | 轻量级 printf 实现 | MIT |
| [picolibc](https://github.com/picolibc/picolibc) | 嵌入式 C 标准库，含 Cortex-M0/M3/M4/M7/M23/M33/M55 预编译库 | BSD |

## 许可

本项目为原创代码，第三方代码遵循各自许可证。

## 模块移植指南

本章按移植顺序逐一说明 ek_utils 全部 17 个模块的集成方式：需要的源文件、依赖关系、配置宏、用户需实现的接口、链接脚本变更、初始化方式和注意事项。

### 移植总览

|序号|模块|类型|关键移植点|
|---|---|---|---|
|1|ek_err|基础层|零移植，纯头文件|
|2|ek_def|基础层|零移植，编译器自动适配|
|3|ek_conf_internal|基础层|用户创建 `ek_conf.h` 即可|
|4|ek_io|核心服务|实现 `_ek_io_fputc()` — 单字符输出|
|5|ek_log|核心服务|实现 `_ek_log_get_tick()` — 系统时间戳|
|6|ek_assert|核心服务|tiny 模式零移植；full 模式依赖 `ek_log`|
|7|ek_heap|核心服务|决定堆段位置（`EKCFG_HEAP_SECTION`）|
|8|ek_list|数据结构|零移植，纯头文件|
|9|ek_vec|数据结构|纯头文件，依赖 `ek_heap`|
|10|ek_ringbuf|数据结构|依赖 `ek_heap`（动态分配）|
|11|ek_stack|数据结构|依赖 `ek_heap`（动态分配）|
|12|ek_str|数据结构|依赖 `ek_heap` + `ek_io`（格式化）|
|13|ek_export|系统服务|**必须修改链接脚本**（`.ek_export_fn` 段）|
|14|ek_evoke|系统服务|仅裸机；实现 5 个弱函数（临界区/定时器/睡眠）|
|15|ek_picothread|系统服务|无弱函数，纯调度器；主循环需配合定时器|
|16|ek_picothread_sem|系统服务|依赖 `ek_picothread`，子模块无额外移植|
|17|ek_picothread_msg|系统服务|依赖 `ek_picothread` + `ek_ringbuf`|

---

### ek_err — 错误码体系

**功能概述**：统一的错误码类型（`ek_err_t`，`uint8_t`）和错误传播宏，提供 40+ 预定义错误码。

**源文件**：
- `ek_utils/inc/ek_err.h`（纯头文件，无 .c）

**依赖**：
- ek_def（`uint8_t`、`__EK_STATIC_INLINE`）

**配置宏**：无 — 始终可用，无需配置开关。

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：无需初始化。

**使用示例**：
```c
#include "ek_err.h"

ek_err_t do_something(void) {
    if (fail) return EK_ERR_NOMEM;
    return EK_ERR_NONE;
}

// 错误传播
EK_ERR_RETURN(do_something());
EK_ERR_GOTO(do_something(), cleanup);

// 错误码转字符串
const char *msg = ek_strerror(EK_ERR_TIMEOUT); // "Timeout"
```

---

### ek_def — 跨编译器兼容层

**功能概述**：跨编译器宏定义（`__EK_WEAK`/`__EK_PACKED`/`__EK_INLINE` 等）和通用工具宏（`EK_ARRAY_LEN`/`EK_CLAMP`/`EK_FREQ_M` 等）。

**源文件**：
- `ek_utils/inc/ek_def.h`（纯头文件，无 .c）

**依赖**：
- `<stdlib.h>`、`<stdint.h>`、`<inttypes.h>`（标准 C 头文件）
- ek_conf_internal（决定是否包含 `<stdarg.h>`）

**配置宏**：无 — 始终可用。宏值根据编译器自动选择（GCC/ARMCC5/ARMCC6/其他）。

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：无需初始化。

**注意事项**：
- 定义了 `bool`/`true`/`false`（`#include <stdbool.h>`）用于 `ek_list` 等模块
- `CRLF` 宏在非 Linux 平台为 `\r\n`，可在 `ek_conf.h` 中覆盖

---

### ek_conf_internal — 配置系统

**功能概述**：配置入口——引入用户 `ek_conf.h`，填充未定义宏的默认值，执行依赖校验。

**源文件**：
- `ek_utils/inc/ek_conf_internal.h`（纯头文件，无 .c）
- **用户创建**：`Core/Inc/ek_conf.h`

**依赖**：无（ek_conf_internal 是配置层起点，其他模块都依赖它）。

**配置宏**：所有 `EKCFG_*` 宏均有默认值（详见"配置宏完整列表"章节）。

**用户需实现的接口**：无 — 只需创建 `ek_conf.h` 文件。

**链接脚本变更**：无。

**初始化**：无需初始化 — 配置在编译期解析为常量。

**注意事项**：
- **用户必须创建 `ek_conf.h`**，否则 `ek_conf_internal.h` 中的 `#include "ek_conf.h"` 会编译失败
- 复制 `ek_utils/ek_conf_template.h` 改名为 `ek_conf.h`，仅保留需要覆盖的宏即可
- 依赖校验规则：`picolibc=1` 自动关闭 lwprintf；`evoke=1` 且 `RTOS=1` 编译报错；sem/msg 开启但 picothread 未开时报错

---

### ek_io — IO 输出封装

**功能概述**：封装底层 printf 输出，支持 lwprintf 和 picolibc 两种后端，提供统一的 `ek_printf` 宏。

**源文件**：
- `ek_utils/inc/ek_io.h`
- `ek_utils/src/ek_io.c`
- lwprintf 后端：`ek_utils/third_party/lwprintf/lwprintf.c`
- picolibc 后端：`ek_utils/third_party/picolibc/*.a`（预编译库）

**依赖**：
- ek_conf_internal、ek_def
- lwprintf 模式：需要 `lwprintf.h` 和 `lwprintf.c`
- picolibc 模式：需要 `<stdio.h>` 和预编译 `libpicolibc.a`

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_IO_LWPRTF` | 0 | 使用 lwprintf 后端（与 PICOLIBC 互斥） |
| `EKCFG_PICOLIBC` | 1 | 使用 picolibc 后端（优先级高于 LWPRTF） |

**用户需实现的接口**：
- `int _ek_io_fputc(int ch)` — 底层单字符输出（UART 发送等）

```c
EK_IO_FPUTC()
{
    bsp_uart_send_data(ch);
    return ch;
}
```

**链接脚本变更**：无。

**初始化**：`ek_io_init()` — lwprintf 模式调用 `lwprintf_init()`；picolibc 模式为空函数；也可通过 `EK_EXPORT_COMPONENTS` 自动调用。

**注意事项**：
- `EKCFG_PICOLIBC=1` 时 `EKCFG_IO_LWPRTF` 被强制设为 0（ek_conf_internal 自动处理）
- picolibc 模式下 `_ek_io_fputc` 返回 `int` 类型（`EOF` 表示错误）；lwprintf 模式下返回 `void`

---

### ek_log — 分级日志

**功能概述**：多级日志输出（DEBUG/INFO/WARN/ERROR），支持 ANSI 彩色、时间戳、文件名和行号。

**源文件**：
- `ek_utils/inc/ek_log.h`
- `ek_utils/src/ek_log.c`

**依赖**：
- ek_io、ek_def、ek_conf_internal

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_LOG` | 1 | 日志模块总开关 |
| `EKCFG_LOG_DEBUG` | 1 | 启用 DEBUG 级别（关闭可减小代码体积） |
| `EKCFG_LOG_COLOR` | 1 | 启用 ANSI 彩色输出 |
| `EKCFG_LOG_BUF_SIZE` | 256 | 单条日志缓冲区大小（字节） |

**用户需实现的接口**：
- `uint32_t _ek_log_get_tick(void)` — 返回系统时间戳（毫秒）

```c
EK_LOG_GET_TICK()
{
    return HAL_GetTick();
}
```

**链接脚本变更**：无。

**初始化**：无需显式初始化。在每个使用日志的 `.c` 文件开头使用 `EK_LOG_FILE_TAG("filename.c")` 声明文件标签。

**使用示例**：
```c
EK_LOG_FILE_TAG("main.c");

EK_LOG_DEBUG("value = %d", val);
EK_LOG_INFO("system started");
EK_LOG_WARN("voltage low: %dmV", mv);
EK_LOG_ERROR("fatal: code=%d", err);
```

---

### ek_assert — 断言

**功能概述**：提供两种断言模式——tiny（死循环，适合资源受限环境）和 full（输出文件名/行号/表达式，依赖 ek_log）。

**源文件**：
- `ek_utils/inc/ek_assert.h`
- `ek_utils/src/ek_assert.c`（仅 full 模式需要）

**依赖**：
- ek_conf_internal、ek_def
- full 模式：依赖 ek_log（`EKCFG_ASSERT_LOG=1` 时输出日志）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_ASSERT` | 1 | 断言模块总开关 |
| `EKCFG_ASSERT_TINY` | 1 | 使用轻量级断言（死循环）；0=full 模式 |
| `EKCFG_ASSERT_LOG` | 1 | 断言失败时输出日志（需要 EKCFG_LOG） |

**用户需实现的接口**：无 — `ek_assert_fault()` 有默认弱实现（输出信息后死循环）。

**链接脚本变更**：无。

**初始化**：无需初始化。通过 `ek_assert_param(expr)` 宏在代码中直接使用。

**注意事项**：
- tiny 模式（默认）：断言失败直接 `while(1)` 死循环，零 RAM/ROM 开销
- full 模式：会调用 `ek_assert_fault()`，可覆盖此函数实现自定义处理（如复位芯片）
- 所有 ek_utils 内部函数参数检查均使用 `ek_assert_param`

---

### ek_heap — 内存堆管理

**功能概述**：基于 TLSF（O(1) 分配/释放）的内存堆管理，支持多内存池，提供 `ek_malloc`/`ek_free`/`ek_realloc` 和 `_safely` 安全释放宏。

**源文件**：
- `ek_utils/inc/ek_heap.h`
- `ek_utils/src/ek_heap.c`
- `ek_utils/third_party/tlsf/tlsf.c`
- `ek_utils/third_party/tlsf/tlsf.h`

**依赖**：
- ek_def、ek_conf_internal
- ek_export（`ek_heap_init` 通过 `EK_EXPORT_EARLIEST(fn, 0)` 自动初始化）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_HEAP_TLSF` | 1 | 使用 TLSF 分配器 |
| `EKCFG_HEAP_SIZE` | 30×1024 | 默认堆大小（字节） |
| `EKCFG_HEAP_SECTION` | 未定义 | 堆所在链接器段（如 `".tcmram"`），不定义则放 `.bss` |

**用户需实现的接口**（仅在 `EKCFG_HEAP_TLSF=0` 时）：
- `void *ek_malloc(size_t size)`
- `void ek_free(void *ptr)`
- `void *ek_realloc(void *ptr, size_t size)`

**链接脚本变更**：如果定义 `EKCFG_HEAP_SECTION`，需确保链接脚本中有对应的段（如 `.tcmram`）。默认无需变更。

**初始化**：`ek_heap_init()` 通过 `EK_EXPORT_EARLIEST(fn, 0)`（level=0）自动初始化，前提是已启用 `EKCFG_EXPORT=1` 并配置链接脚本段。否则需在 `main()` 起始手动调用。

**注意事项**：
- TLSF 内部管理开销约 768 字节（默认配置 `FL_INDEX_MAX=24, SL_INDEX_COUNT_LOG2=3`），可根据实际内存池大小调整（见 `ek_heap.c` 顶部注释）
- `ek_free_safely(&ptr)` 释放后自动将 `ptr` 置 NULL
- `ek_heap_add_pool()` 支持运行时添加额外内存池（如 SDRAM 区域）

---

### ek_list — 双向循环链表

**功能概述**：Linux 内核风格的侵入式双向循环链表，所有操作 O(1)，零额外内存分配。

**源文件**：
- `ek_utils/inc/ek_list.h`（纯头文件，无 .c）

**依赖**：
- ek_def（`bool`/`offsetof`/`__EK_STATIC_INLINE`）
- ek_conf_internal（`EKCFG_LIST` 守卫）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_LIST` | 0 | 模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：`ek_list_init(&head)` 初始化链表头。

**使用示例**：
```c
typedef struct {
    int data;
    ek_list_node_t node; // 嵌入链表节点
} my_item_t;

ek_list_node_t head;
ek_list_init(&head);

my_item_t a = { .data = 42 };
ek_list_insert_tail(&head, &a.node);

// 遍历
ek_list_node_t *pos;
ek_list_foreach(pos, &head) {
    my_item_t *item = ek_list_container(pos, my_item_t, node);
}
```

**注意事项**：
- 节点必须嵌入用户结构体，通过 `ek_list_container` 反查父结构体地址
- `ek_list_remove` 后将节点 prev/next 置 NULL，可用于检测节点是否在链表中
- 删除遍历时使用 `ek_list_foreach_safe`

---

### ek_vec — 动态数组

**功能概述**：类型安全的动态数组，通过宏生成类型化结构体和操作，自动扩容。

**源文件**：
- `ek_utils/inc/ek_vec.h`（纯头文件，无 .c）

**依赖**：
- ek_def、ek_heap（`ek_realloc`）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_VEC` | 0 | 模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：`ek_vec_init(v)` 初始化（零开销，仅将指针和计数器置零）。

**使用示例**：
```c
EK_VEC_IMPLEMENT(int);          // 生成 ek_vec_int_t 类型
ek_vec_t(int) v;                // 声明数组
ek_vec_init(v);                 // 初始化
ek_vec_append(v, 42);           // 追加
ek_vec_remove(v, 0);            // 按索引移除
ek_vec_destroy(v);              // 释放内存
```

**注意事项**：
- 扩容策略：容量 <32 时翻倍，≥32 时加 1/2
- `ek_vec_append` 内部分配失败时 `break` 跳出（不追加），需自行检测 `v.amount` 变化
- `EK_VEC_IMPLEMENT` 必须在全局作用域使用

---

### ek_ringbuf — 环形缓冲区

**功能概述**：任意类型的环形缓冲区（通用版 `ek_ringbuf_t` + SPSC 版 `ek_ringbuf_spsc_t`），RTOS 模式下通用版含锁。

**源文件**：
- `ek_utils/inc/ek_ringbuf.h`
- `ek_utils/src/ek_ringbuf.c`

**依赖**：
- ek_err、ek_conf_internal
- ek_heap（动态分配）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_RINGBUF` | 0 | 通用环形缓冲区 |
| `EKCFG_RINGBUF_SPSC` | 0 | SPSC 无锁环形缓冲区 |

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：
```c
// 通用版：item_amount = 实际可存元素数
ek_ringbuf_t *rb = ek_ringbuf_create(sizeof(my_data_t), 10);

// SPSC 版：item_amount = 底层槽位数，实际可存 item_amount - 1 个元素
ek_ringbuf_spsc_t *rb_spsc = ek_ringbuf_create_spsc(sizeof(my_data_t), 11);
```

**注意事项**：
- 通用版在 RTOS 模式下使用 `EK_LOCKUP/EK_UNLOCK` 保护
- SPSC 版无锁，适合 ISR→主循环 或 单生产者单消费者场景（evoke 内部使用）
- SPSC 版满判据：`(write_idx + 1) % cap == read_idx`
- 使用 `ek_ringbuf_destroy_safely(&rb)` 安全释放

---

### ek_stack — 通用栈

**功能概述**：LIFO 栈，支持任意数据类型，动态分配底层缓冲区。

**源文件**：
- `ek_utils/inc/ek_stack.h`
- `ek_utils/src/ek_stack.c`

**依赖**：
- ek_err、ek_conf_internal
- ek_heap（动态分配）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_STACK` | 0 | 模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：无。

**初始化**：
```c
ek_stack_t *sk = ek_stack_create(sizeof(int), 20);
ek_stack_push(sk, &val);
ek_stack_pop(sk, &val);
ek_stack_destroy_safely(&sk);
```

**注意事项**：
- RTOS 模式下使用锁保护
- 所有操作按 `item_size` 字节复制数据（值语义）

---

### ek_str — 动态字符串

**功能概述**：自动扩容的动态字符串，支持追加、切片、格式化输出、比较、翻转。

**源文件**：
- `ek_utils/inc/ek_str.h`
- `ek_utils/src/ek_str.c`

**依赖**：
- ek_err、ek_conf_internal
- ek_heap（`ek_malloc`/`ek_realloc`）
- ek_io（`ek_vsnprintf` — `ek_str_append_fmt` 需要）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_STR` | 0 | 模块开关 |

**用户需实现的接口**：无（间接依赖 `ek_io`，需实现 `_ek_io_fputc` 以支持格式化整数输出）。

**链接脚本变更**：无。

**初始化**：
```c
ek_str_t *s = ek_str_create("hello");
ek_str_append(s, " world");
ek_str_append_fmt(s, "(%d)", 42);
const char *cs = ek_str_get_cstring(s); // "hello world(42)"
ek_str_free(s);
```

**注意事项**：
- 扩容策略：当前容量的 1.5 倍
- `ek_str_get_cstring` 返回内部缓冲区指针，不要手动释放
- `ek_str_slice` 创建新字符串对象，需单独释放

---

### ek_export — 自动初始化

**功能概述**：基于链接器段的函数自动导出和初始化，类似 Linux `initcall`。通过 `EK_EXPORT_LEVEL(fn, level, order)` 将函数指针放入 `.ek_export_fn` 段，`ek_export_init()` 运行时按 `level`（层级）和 `order`（层内优先级）排序后逐个调用——排序通过 `qsort` 在运行时完成，不依赖链接器行为。

**源文件**：
- `ek_utils/inc/ek_export.h`
- `ek_utils/src/ek_export.c`

**依赖**：
- ek_conf_internal

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_EXPORT` | 0 | 模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：**必须添加 `.ek_export` 段**，否则导出的初始化函数不会被执行：

```ld
/* 在链接脚本 (.ld) 中添加 */
.ek_export :
{
    . = ALIGN(4);
    _ek_export_fn_start = .;
    KEEP(*(.ek_export_fn*))
    . = ALIGN(4);
    _ek_export_fn_end = .;
} > FLASH
```

**初始化**：在 `main()` 起始调用 `ek_export_init()`：

```c
int main(void) {
    ek_export_init(); // 按 level → order 排序后调用所有导出函数
    // ...
}
```

**使用示例**：
```c
void my_init(void) {
    // 初始化代码
}
EK_EXPORT_LEVEL(my_init, 3, 0); // level=3（应用层），order=0

// 预定义层级别名（等价于 EK_EXPORT_LEVEL(fn, level, order)）
EK_EXPORT_EARLIEST(heap_init, 0);    // level=0 — 堆初始化
EK_EXPORT_HARDWARE(hw_init, 0);      // level=1 — 硬件初始化
EK_EXPORT_COMPONENTS(comp_init, 0);  // level=2 — 组件初始化
EK_EXPORT_APP(app_init, 0);          // level=3 — 应用初始化
EK_EXPORT_USER(user_init, 0);        // level=4 — 用户初始化
```

**注意事项**：
- 排序规则：先按 `level`（层级，0–4）升序，同层级内按 `order`（层内优先级）升序；通过 `qsort` 在运行时完成
- `KEEP(*(.ek_export_fn*))` 保证即使 `--gc-sections` 也不会移除导出函数
- `EKCFG_EXPORT=0` 时所有导出宏展开为空
- ek_utils 内部模块（heap/evoke/picothread）自身也通过此机制自动初始化

---

### ek_evoke — 事件驱动调度器

**功能概述**：协作式事件驱动任务调度器。ISR 通过 SPSC ringbuf FIFO 向主循环推送事件请求（发布/广播/延迟），主循环从 FIFO 取出请求、执行就绪任务、管理延迟事件池和睡眠策略。仅裸机可用。

**源文件**：
- `ek_utils/inc/ek_evoke.h`
- `ek_utils/src/ek_evoke.c`

**依赖**：
- ek_err、ek_def、ek_ringbuf、ek_heap、ek_export、ek_log、ek_assert
- ek_ringbuf_spsc（ISR FIFO 通信）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_EVOKE` | 0 | 模块开关（需 `EKCFG_RTOS=0`） |
| `EKCFG_EVOKE_MIN_DEEPSLEEP_TICK` | 10 | 进入深度睡眠的最小空闲 tick |

**编译期常量（可覆盖）**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EK_EVOKE_MAX_ISR_REQ` | 10 | ISR 请求队列容量 |
| `EK_EVOKE_MAX_DEFER_REQ` | 10 | 延迟请求池容量 |

**用户需实现的接口（5 个弱函数）**：

| 函数 | 作用 | 典型实现 |
|------|------|---------|
| `ek_evoke_enter_critical()` | 进入临界区 | `__disable_irq()` |
| `ek_evoke_exit_critical()` | 退出临界区 | `__enable_irq()` |
| `ek_evoke_set_timer(uint32_t xtick)` | 设置单次定时器 | 设置硬件定时器在 xtick 后触发中断，中断中调用 `ek_evoke_delay_timer_callback()` |
| `ek_evoke_light_sleep()` | 浅睡眠 | `__WFI()` |
| `ek_evoke_deep_sleep()` | 深度睡眠 | 进入低功耗模式（STOP/STANDBY），需配置唤醒源 |

**链接脚本变更**：无（但如果使用 `EK_EXPORT_LEVEL` 自动初始化则需配置 `.ek_export` 段）。

**初始化**：`ek_evoke_init()` 通过 `EK_EXPORT_COMPONENTS` 自动初始化，或手动调用。初始化后通过 `ek_evoke_event_loop()` 进入主循环（永不返回）。

**典型主循环模式**：
```c
int main(void) {
    ek_export_init();         // 自动初始化 heap → evoke

    // 创建任务和事件
    ek_evoke_task_handle_t tsk = ek_evoke_task_create("led", led_cb, NULL);
    ek_evoke_event_handle_t evt = ek_evoke_event_create("timer", 0);
    ek_evoke_event_subscribe(tsk, evt);

    ek_evoke_event_loop();   // 永不返回
}

// ISR 中发布事件
void TIMER_IRQHandler(void) {
    ek_evoke_event_publish_from_isr(evt, NULL);
}
```

**注意事项**：
- `ek_evoke_event_loop()` 永不返回，调用前必须完成所有初始化
- ISR 中的 `*_from_isr` 函数内部使用 `ek_evoke_enter/exit_critical` 保护 FIFO 写入
- 延迟事件通过 `ek_evoke_set_timer()` 设置硬件定时器唤醒
- 睡眠锁（`ek_evoke_sleep_lock/unlock`）用于在关键操作期间禁止深度睡眠

---

### ek_picothread — 微线程调度器

**功能概述**：基于 protothread 的协作式微线程调度器。通过 `EK_PT_BEGIN/YEILD/END` 宏在单函数内实现协程式逻辑，支持优先级调度和定时阻塞。

**源文件**：
- `ek_utils/inc/ek_picothread.h`
- `ek_utils/src/ek_picothread.c`

**依赖**：
- ek_err、ek_list、ek_heap、ek_assert、ek_export
- ek_ringbuf（仅在启用 `picothread_msg` 时需要）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_PICOTHREAD` | 0 | 微线程调度器总开关 |
| `EKCFG_PICOTHREAD_SEM` | 0 | 信号量子模块（依赖 PICOTHREAD） |
| `EKCFG_PICOTHREAD_MSG` | 0 | 消息队列子模块（依赖 PICOTHREAD + RINGBUF） |

**用户需实现的接口**：无 — 无弱函数，纯调度器。

**链接脚本变更**：使用 `EK_EXPORT_LEVEL` 自动初始化则需配置 `.ek_export` 段。

**初始化**：`ek_pt_init()` 通过 `EK_EXPORT_COMPONENTS`（level=2）自动初始化，或手动调用。

**主循环模式**：

`ek_pt_schedule(now)` 返回 `uint32_t`，有三种情况：

| 返回值 | 含义 | 主循环行为 |
|--------|------|-----------|
| `== now` | 有就绪任务，不应睡眠 | 继续循环执行调度 |
| `> now` | 返回最近阻塞任务的唤醒 tick | 设置定时器在 `ret - now` tick 后唤醒 |
| `== 0` | 无任何任务 | 可深度睡眠（等待外部中断唤醒） |

```c
static void led_task(ek_pt_handle_t pt, void *arg) {
    EK_PT_BEGIN(pt);
    while (1) {
        led_toggle();
        EK_PT_DELAY(500); // 阻塞 500 ticks
    }
    EK_PT_END(pt);
}

int main(void) {
    ek_export_init();
    ek_pt_create("led", led_task, 0, NULL);

    while (1) {
        uint32_t now = get_tick();
        uint32_t next = ek_pt_schedule(now);
        if (!next) {
            enter_deep_sleep(); // 无任务，深度睡眠
        } else if (next > now) {
            set_timer_once(next - now); // 定时唤醒
            enter_light_sleep();
        }
        // next == now：继续循环
    }
}
```

**注意事项**：
- `EK_PT_BEGIN`/`EK_PT_END` 必须在回调函数体内配对使用，利用 `switch-case` 实现协程恢复
- `EK_PT_YEILD` 保存行号后 `return`，下次调度从此处恢复 — 确保局部变量不跨 `YEILD` 保存状态（用 `static` 变量或结构体成员）
- 优先级数值越小越高，在就绪链表头插入
- 阻塞链表按唤醒 tick（绝对时间）排序，tick 相同时按优先级排序
- `ek_pt_destroy` 不能在任务正在运行（`s_cur_pt`）时销毁自身

---

### ek_picothread_sem — 微线程信号量

**功能概述**：微线程间的同步原语，支持计数信号量和超时等待。

**源文件**：无需额外文件 — 实现在 `ek_picothread.h`/`.c` 中，由 `EKCFG_PICOTHREAD_SEM` 守卫控制编译。

**依赖**：
- ek_picothread（`EKCFG_PICOTHREAD=1`）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_PICOTHREAD_SEM` | 0 | 信号量子模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：无（继承 ek_picothread 的配置）。

**初始化**：
```c
ek_pt_sem_handle_t sem = ek_pt_sem_create(1); // 初始计数 = 1（二值信号量）
```

**使用示例**：
```c
static void producer(ek_pt_handle_t pt, void *arg) {
    ek_pt_sem_handle_t sem = (ek_pt_sem_handle_t)arg;
    ek_err_t err;
    EK_PT_BEGIN(pt);
    // 获取信号量，超时 1000 ticks
    EK_PT_SEM_TAKE(sem, 1000, err);
    if (err == EK_ERR_NONE) {
        // 临界区操作
    }
    EK_PT_SEM_GIVE(sem);
    EK_PT_END(pt);
}
```

**注意事项**：
- `EK_PT_SEM_TAKE` 在信号量不可用时阻塞当前任务，超时后 `err == EK_ERR_TIMEOUT`
- `EK_PT_SEM_GIVE` 优先唤醒等待队列中优先级最高的任务
- `ek_pt_sem_destroy` 会唤醒所有等待任务，`wait_result` 设为 `EK_ERR_ABORTED`

---

### ek_picothread_msg — 微线程消息队列

**功能概述**：微线程间的消息传递机制，内部复用 `ek_ringbuf_t` 存储消息数据，支持发送/接收超时等待。

**源文件**：无需额外文件 — 实现在 `ek_picothread.h`/`.c` 中，由 `EKCFG_PICOTHREAD_MSG` 守卫控制编译。

**依赖**：
- ek_picothread（`EKCFG_PICOTHREAD=1`）
- ek_ringbuf（`EKCFG_RINGBUF=1`）

**配置宏**：
| 宏 | 默认值 | 说明 |
|----|--------|------|
| `EKCFG_PICOTHREAD_MSG` | 0 | 消息队列子模块开关 |

**用户需实现的接口**：无。

**链接脚本变更**：无（继承 ek_picothread 的配置）。

**初始化**：
```c
ek_pt_msg_handle_t msg = ek_pt_msg_create(sizeof(my_data_t), 8); // 8 条消息容量
```

**使用示例**：
```c
typedef struct { int id; float val; } sensor_data_t;
static void sender(ek_pt_handle_t pt, void *arg) {
    ek_pt_msg_handle_t mq = (ek_pt_msg_handle_t)arg;
    ek_err_t err;
    sensor_data_t data = { .id = 1, .val = 3.14f };
    EK_PT_BEGIN(pt);
    EK_PT_MSG_SEND(mq, &data, 500, err);
    if (err == EK_ERR_TIMEOUT) { /* 超时处理 */ }
    EK_PT_END(pt);
}

static void receiver(ek_pt_handle_t pt, void *arg) {
    ek_pt_msg_handle_t mq = (ek_pt_msg_handle_t)arg;
    ek_err_t err;
    sensor_data_t data;
    EK_PT_BEGIN(pt);
    EK_PT_MSG_RECV(mq, &data, 1000, err);
    if (err == EK_ERR_NONE) { /* 处理 data */ }
    EK_PT_END(pt);
}
```

**注意事项**：
- 消息队列满时发送任务阻塞在 send_wait 队列；空时接收任务阻塞在 recv_wait 队列
- `ek_pt_msg_destroy` 会唤醒所有等待任务，`wait_result` 设为 `EK_ERR_ABORTED`
- 数据按 `item_size` 字节复制（值语义），与 `ek_ringbuf` 行为一致
