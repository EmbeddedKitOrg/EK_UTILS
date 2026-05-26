/**
 * @file ek_conf.h
 * @brief EmbeddedKit 全局配置文件
 * @author N1netyNine99
 *
 * 此文件定义了整个 EmbeddedKit 框架的全局配置宏。
 * 所有配置宏统一使用 EKCFG_ 前缀，正逻辑命名（1=启用，0=禁用）。
 * 本文件是唯一的配置来源，各模块不应自行定义默认值。
 */

#ifndef EK_CONF_H
#define EK_CONF_H

/* ========================================================================
 * Section 1: 平台/运行环境
 * ======================================================================== */

#define EKCFG_RTOS      (0) /**< 是否使用 RTOS (0=裸机) */
#define EKCFG_PICOLIBC  (0) /**< 是否使用 picolibc 替代标准 libc */
#define EKCFG_IO_LWPRTF (1) /**< IO 是否使用 lwprintf (1=使用, 0=不使用) */

/* ========================================================================
 * Section 2: 核心服务开关
 * ======================================================================== */

#define EKCFG_EXPORT (0) /**< 自动初始化机制 */
#define EKCFG_ASSERT (1) /**< 断言模块 */
#define EKCFG_LOG    (1) /**< 日志模块 */

/* ========================================================================
 * Section 3: 数据结构开关
 * ======================================================================== */

#define EKCFG_STR          (1) /**< 动态字符串 */
#define EKCFG_LIST         (1) /**< 双向循环链表 */
#define EKCFG_VEC          (1) /**< 动态数组 */
#define EKCFG_RINGBUF      (1) /**< 通用环形缓冲区 */
#define EKCFG_RINGBUF_SPSC (1) /**< SPSC 无锁环形缓冲区 */
#define EKCFG_STACK        (1) /**< 通用栈 */
#define EKCFG_EVOKE        (1) /**< 事件驱动调度器 */

/* ========================================================================
 * Section 4: 模块子配置
 * ======================================================================== */

#define EKCFG_HEAP_TLSF    (1) /**< 内存堆使用 TLSF 分配器 (1=TLSF, 0=自定义) */
#define EKCFG_HEAP_SIZE    (30 * 1024) /**< 内存堆大小（字节） */

#define EKCFG_LOG_DEBUG    (1) /**< 启用 DEBUG 级别日志 */
#define EKCFG_LOG_COLOR    (1) /**< 启用 ANSI 彩色日志 */
#define EKCFG_LOG_BUF_SIZE (256) /**< 日志缓冲区大小（字节） */

#define EKCFG_ASSERT_TINY  (1) /**< 使用轻量级断言模式 */
#define EKCFG_ASSERT_LOG   (1) /**< 断言失败时输出日志 */

/* ========================================================================
 * Section 5: 配置依赖校验
 * ======================================================================== */

#if EKCFG_PICOLIBC == 1
#    define EKCFG_IO_LWPRTF (0) /* picolibc 接管 printf，关闭 lwprintf */
#endif

#if EKCFG_EVOKE == 1 && EKCFG_RTOS == 1
#    error "EKCFG_EVOKE requires EKCFG_RTOS == 0 (bare-metal only)"
#endif

#endif /* EK_CONF_H */