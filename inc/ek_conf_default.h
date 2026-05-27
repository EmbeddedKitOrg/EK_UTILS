/**
 * @file ek_conf_default.h
 * @brief EmbeddedKit 默认配置——所有宏均用 #ifndef 守卫，可由用户配置覆盖
 * @author N1netyNine99
 *
 * 此文件由 ek_conf.h 引入，不要在用户代码中直接 include。
 * 用户只需在自己的 ek_conf.h 中先 #define 需要覆盖的宏，再 include 此文件。
 */

#ifndef EK_CONF_DEFAULT_H
#define EK_CONF_DEFAULT_H

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

#endif /* EK_CONF_DEFAULT_H */
