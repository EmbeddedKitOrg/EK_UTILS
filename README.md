# EmbeddedKit (ek_utils)

面向资源受限 MCU 环境的轻量级嵌入式 C 工具库。提供数据结构、内存管理、日志、事件调度等裸机和 RTOS 开发的通用基础设施。

## 特性

- **零依赖**：无需 RTOS，可直接集成到裸机项目
- **配置驱动**：所有模块通过 `ek_conf.h` 宏开关按需裁剪，编译期零开销
- **跨编译器**：支持 GCC、ARM Compiler 5 (armcc)、ARM Compiler 6 (armclang)
- **弱函数扩展**：平台相关接口通过 `__EK_WEAK` 弱符号声明，用户提供强函数覆盖
- **安全释放**：`_safely` 系列宏在释放后自动将指针置 NULL，防止悬空指针

## 项目结构

```
ek_utils/
├── inc/                         # 头文件
│   ├── ek_conf.h                # 全局配置（所有模块开关和参数）
│   ├── ek_def.h                 # 跨编译器兼容层（__EK_WEAK / __EK_PACKED 等）
│   ├── ek_err.h                 # 错误码定义（ek_err_t）和错误处理宏
│   ├── ek_assert.h              # 断言模块
│   ├── ek_log.h                 # 分级日志模块
│   ├── ek_io.h                  # IO 输出封装（lwprintf / picolibc / 空实现）
│   ├── ek_heap.h                # 内存堆管理（基于 TLSF）
│   ├── ek_list.h                # 双向循环链表（纯头文件）
│   ├── ek_vec.h                 # 类型安全动态数组（纯头文件，宏生成类型）
│   ├── ek_ringbuf.h             # 环形缓冲区（通用 + SPSC 无锁变体）
│   ├── ek_stack.h               # 通用 LIFO 栈
│   ├── ek_str.h                 # 动态字符串
│   ├── ek_export.h              # 函数自动导出初始化（类似 Linux initcall）
│   └── ek_evoke.h               # 协作式事件驱动任务调度器
├── src/                         # 源文件
│   ├── ek_assert.c
│   ├── ek_evoke.c
│   ├── ek_export.c
│   ├── ek_heap.c
│   ├── ek_io.c
│   ├── ek_log.c
│   ├── ek_picolibc_port.c        # picolibc 适配层
│   ├── ek_ringbuf.c
│   ├── ek_stack.c
│   └── ek_str.c
├── third_party/                 # 第三方代码
│   ├── tlsf/                    # TLSF 实时内存分配器（O(1) 分配/释放）
│   ├── lwprintf/                # 轻量级 printf 实现
│   └── picolibc/                # 轻量级 C 标准库（含 Cortex-M 预编译库）
├── cmake/
│   └── gcc-arm-none-eabi.cmake  # ARM 交叉编译工具链配置
└── CMakeLists.txt               # CMake 构建配置
```

## 模块清单

| 模块 | 配置宏 | 类型 | 说明 |
|------|--------|------|------|
| `ek_assert` | `EKCFG_ASSERT` | 核心服务 | 断言：tiny 模式（死循环）和 full 模式（输出详情） |
| `ek_log` | `EKCFG_LOG` | 核心服务 | 分级日志（DEBUG/INFO/WARN/ERROR），支持 ANSI 彩色和时间戳 |
| `ek_io` | `EKCFG_IO_LWPRTF` / `EKCFG_PICOLIBC` | 核心服务 | IO 输出封装，支持 lwprintf 或 picolibc 两种后端 |
| `ek_heap` | `EKCFG_HEAP_TLSF` | 核心服务 | 基于 TLSF 的内存堆管理，支持多内存池 |
| `ek_err` | 始终包含 | 基础层 | 错误码体系（ek_err_t），含 45+ 错误码和错误传播宏 |
| `ek_list` | `EKCFG_LIST` | 数据结构 | Linux 内核风格双向循环链表（纯头文件，0 额外内存分配） |
| `ek_vec` | `EKCFG_VEC` | 数据结构 | 类型安全动态数组（纯头文件，宏生成类型） |
| `ek_ringbuf` | `EKCFG_RINGBUF` | 数据结构 | 通用环形缓冲区（RTOS 模式下含锁） |
| `ek_ringbuf_spsc` | `EKCFG_RINGBUF_SPSC` | 数据结构 | SPSC 单生产者单消费者无锁环形缓冲区 |
| `ek_stack` | `EKCFG_STACK` | 数据结构 | 通用 LIFO 栈 |
| `ek_str` | `EKCFG_STR` | 数据结构 | 自动扩容的动态字符串 |
| `ek_export` | `EKCFG_EXPORT` | 系统服务 | 基于链接器段的自动初始化（需配合链接脚本） |
| `ek_evoke` | `EKCFG_EVOKE` | 系统服务 | 协作式事件驱动调度器（仅裸机，`EKCFG_RTOS=0` 时可用） |

## 快速上手

### 方式一：CMake 集成（推荐）

```bash
# 配置（必须指定 MCU 平台，如 cm4）
cmake -DPLATFORM=cm4 -G Ninja -B build

# 构建静态库
ninja -C build
# → build/libek_utils.a

# 使用 picolibc 作为标准库（可选）
cmake -DPLATFORM=cm4 -DPICOLIBC=ON -G Ninja -B build
ninja -C build

# 对象库模式（仅编译 .o 文件，不打包 .a）
cmake -DPLATFORM=cm4 -DOBJ_LIB=ON -G Ninja -B build
ninja -C build
```

### 支持的 MCU 平台

| PLATFORM | CPU | FPU |
|----------|-----|-----|
| cm0 | Cortex-M0 | 无 |
| cm3 | Cortex-M3 | 无 |
| cm4 | Cortex-M4 | fpv4-sp-d16 (hard) |
| cm7 | Cortex-M7 | fpv5-d16 (hard) |
| cm23 | Cortex-M23 | 无 |
| cm33 | Cortex-M33 | fpv5-sp-d16 (hard) |
| cm55 | Cortex-M55 | fpv5-auto (hard) |

### 方式二：源文件直接集成

将以下内容加入你的项目编译系统：

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

### 方式三：在 CMake 项目中作为子目录引入

```cmake
# 在你的 CMakeLists.txt 中
add_subdirectory(path/to/ek_utils)
target_link_libraries(your_app ek_utils)
```

### 对象库模式使用

如果 ek_utils 编译为对象库（`OBJ_LIB=ON`），在你的 CMake 中：

```cmake
add_subdirectory(path/to/ek_utils)
target_link_libraries(your_app $<TARGET_OBJECTS:ek_utils>)
# 对象库模式下需自行处理 picolibc 链接
```

## 配置说明

所有配置集中在 `inc/ek_conf.h`，修改宏值即可启用/禁用模块：

```c
/* 平台/运行环境 */
#define EKCFG_RTOS      (0)     // 是否使用 RTOS
#define EKCFG_PICOLIBC  (1)     // 是否使用 picolibc（自动关闭 lwprintf）
#define EKCFG_IO_LWPRTF (0)     // IO 是否使用 lwprintf

/* 核心服务（1=启用，0=禁用） */
#define EKCFG_EXPORT (0)
#define EKCFG_ASSERT (1)
#define EKCFG_LOG    (1)

/* 数据结构（1=启用，0=禁用） */
#define EKCFG_STR          (1)
#define EKCFG_LIST         (1)
#define EKCFG_VEC          (1)
#define EKCFG_RINGBUF      (1)
#define EKCFG_RINGBUF_SPSC (1)
#define EKCFG_STACK        (1)
#define EKCFG_EVOKE        (1)

/* 子配置 */
#define EKCFG_HEAP_TLSF    (1)         // 使用 TLSF 分配器
#define EKCFG_HEAP_SIZE    (30 * 1024) // 默认堆大小
#define EKCFG_LOG_DEBUG    (1)         // 启用 DEBUG 日志
#define EKCFG_LOG_COLOR    (1)         // 启用彩色日志
#define EKCFG_LOG_BUF_SIZE (256)       // 日志缓冲区大小
#define EKCFG_ASSERT_TINY  (1)         // 轻量级断言
#define EKCFG_ASSERT_LOG   (1)         // 断言时输出日志
```

### picolibc vs lwprintf

| 场景 | 配置 | 说明 |
|------|------|------|
| 标准 IO (picolibc) | `EKCFG_PICOLIBC=1` | ek_printf → printf，ek_malloc → picolibc malloc |
| 轻量 IO (lwprintf) | `EKCFG_IO_LWPRTF=1, EKCFG_PICOLIBC=0` | ek_printf → lwprintf，适合没有完整 libc 的环境 |

## 设计约定

### 弱函数（Weak Functions）

所有需要平台或用户定制的函数以 `__EK_WEAK` 声明，用户提供同名强函数即可覆盖：

```c
// 需要用户实现的弱函数
__EK_WEAK void _ek_io_fputc(int ch)     // 字符输出（UART 等）
__EK_WEAK uint32_t _ek_log_get_tick(void) // 系统时间戳
__EK_WEAK void ek_evoke_enter_critical(void)  // 进入临界区
__EK_WEAK void ek_evoke_light_sleep(void)     // 浅睡眠（WFI）
```

### 错误码体系

所有非 `void` 返回的操作函数使用 `ek_err_t`（`uint8_t`）返回错误码：

```c
ek_err_t ek_ringbuf_write(ek_ringbuf_t *rb, const void *item);
// 返回 EK_ERR_NONE(0) 成功，EK_ERR_FULL(207) 满，EK_ERR_BUSY(202) 忙

// 错误传播宏
EK_RETURN_IF_ERR(ek_str_append(s, "hello"));  // 非零则直接 return
EK_GOTO_IF_ERR(ek_foo(), cleanup);             // 非零则 goto cleanup
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