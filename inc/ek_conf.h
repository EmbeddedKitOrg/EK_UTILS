/**
 * @file ek_conf.h
 * @brief EmbeddedKit 配置入口
 *
 * 推荐用法（子模块模式）：
 *   在你的项目中创建 ek_conf.h，先 #define 要覆盖的宏，再 include 此文件或将此文件的内容放在最后。
 *   确保你的 include 路径优先级高于 ek_utils/inc/，即可覆盖本文件。
 *
 * 独立构建：
 *   本文件直接引入 ek_conf_default.h，使用全部默认值。
 */

#ifndef EK_CONF_H
#define EK_CONF_H

#include "ek_conf_default.h"

#endif /* EK_CONF_H */
