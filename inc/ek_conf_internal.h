/**
 * @file ek_conf_internal.h
 * @brief EmbeddedKit 内部配置入口——库内所有模块均通过此文件获取配置
 * @author N1netyNine99
 *
 * 本文件负责：
 *   1. 引入用户提供的 ek_conf.h
 *   2. 为用户未定义的 EKCFG_* 宏填充默认值
 *   3. 执行配置依赖校验
 *
 * 用户无需直接 include 或修改此文件。
 * 只需在自己的项目中创建 ek_conf.h（复制仓库根目录的 ek_conf_template.h 并改名），
 * 覆盖所需的宏，其余自动取默认值。
 *
 * 指定用户 ek_conf.h 的两种方式：
 *   A. CMake 参数：cmake -DEK_CONF_PATH="path/to/ek_conf.h"
 *   B. include 优先级：确保你的 ek_conf.h 所在路径先于 ek_utils/inc/ 被搜索到
 */

#ifndef EK_CONF_INTERNAL_H
#define EK_CONF_INTERNAL_H

/*
 * ========================================================================
 * 引入用户配置
 * ========================================================================
 * 方式 A：CMake 传 -DEK_CONF_PATH="xxx" → 编译期宏展开为 include 路径
 * 方式 B：未设置 EK_CONF_PATH → 使用标准 include 搜索，需保证用户路径优先
 */
#ifdef EK_CONF_PATH
#    include EK_CONF_PATH
#else
#    include "ek_conf.h"
#endif

/* ========================================================================
 * 平台/运行环境
 * ======================================================================== */

#ifndef EKCFG_RTOS
#    define EKCFG_RTOS (0) /**< 是否使用 RTOS (0=裸机) */
#endif
#ifndef EKCFG_PICOLIBC
#    define EKCFG_PICOLIBC (1) /**< 是否使用 picolibc 替代标准 libc */
#endif
#ifndef EKCFG_IO_LWPRTF
#    define EKCFG_IO_LWPRTF (0) /**< IO 是否使用 lwprintf (1=使用, 0=不使用) */
#endif

/* ========================================================================
 * 核心服务开关
 * ======================================================================== */

#ifndef EKCFG_EXPORT
#    define EKCFG_EXPORT (0) /**< 自动初始化机制 */
#endif
#ifndef EKCFG_ASSERT
#    define EKCFG_ASSERT (1) /**< 断言模块 */
#endif
#ifndef EKCFG_LOG
#    define EKCFG_LOG (1) /**< 日志模块 */
#endif

/* ========================================================================
 * 数据结构开关
 * ======================================================================== */

#ifndef EKCFG_STR
#    define EKCFG_STR (1) /**< 动态字符串 */
#endif
#ifndef EKCFG_LIST
#    define EKCFG_LIST (1) /**< 双向循环链表 */
#endif
#ifndef EKCFG_VEC
#    define EKCFG_VEC (1) /**< 动态数组 */
#endif
#ifndef EKCFG_RINGBUF
#    define EKCFG_RINGBUF (1) /**< 通用环形缓冲区 */
#endif
#ifndef EKCFG_RINGBUF_SPSC
#    define EKCFG_RINGBUF_SPSC (1) /**< SPSC 无锁环形缓冲区 */
#endif
#ifndef EKCFG_STACK
#    define EKCFG_STACK (1) /**< 通用栈 */
#endif
#ifndef EKCFG_EVOKE
#    define EKCFG_EVOKE (1) /**< 事件驱动调度器 */
#endif
#ifndef EKCFG_PICOTHREAD
#    define EKCFG_PICOTHREAD (1) /**< 微线程 */
#endif

/* ========================================================================
 * 模块子配置
 * ======================================================================== */

#ifndef EKCFG_HEAP_TLSF
#    define EKCFG_HEAP_TLSF (1) /**< 内存堆使用 TLSF 分配器 (1=TLSF, 0=自定义) */
#endif
#ifndef EKCFG_HEAP_SIZE
#    define EKCFG_HEAP_SIZE (30 * 1024) /**< 内存堆大小（字节） */
#endif
#ifndef EKCFG_LOG_DEBUG
#    define EKCFG_LOG_DEBUG (1) /**< 启用 DEBUG 级别日志 */
#endif
#ifndef EKCFG_LOG_COLOR
#    define EKCFG_LOG_COLOR (1) /**< 启用 ANSI 彩色日志 */
#endif
#ifndef EKCFG_LOG_BUF_SIZE
#    define EKCFG_LOG_BUF_SIZE (256) /**< 日志缓冲区大小（字节） */
#endif
#ifndef EKCFG_ASSERT_TINY
#    define EKCFG_ASSERT_TINY (1) /**< 使用轻量级断言模式 */
#endif
#ifndef EKCFG_ASSERT_LOG
#    define EKCFG_ASSERT_LOG (1) /**< 断言失败时输出日志 */
#endif

/* ========================================================================
 * 配置依赖校验（始终执行，不区分配置来源）
 * ======================================================================== */

#if EKCFG_PICOLIBC == 1
#    undef EKCFG_IO_LWPRTF
#    define EKCFG_IO_LWPRTF (0) /* picolibc 接管 printf，关闭 lwprintf */
#endif

#if EKCFG_EVOKE == 1 && EKCFG_RTOS == 1
#    error "EKCFG_EVOKE requires EKCFG_RTOS == 0 (bare-metal only)"
#endif

#endif /* EK_CONF_INTERNAL_H */
