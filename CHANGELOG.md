# 更新日志

本文件记录 [ek_utils](https://github.com/EmbeddedKitOrg/EK_UTILS) 的版本变更。

格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [2.0.0rc] - 2026-08-14

相对 [v1.2.0](https://github.com/EmbeddedKitOrg/EK_UTILS/releases/tag/v1.2.0) 的预发布版本。包含破坏性 API 变更，请勿当作稳定版依赖。

### 新增

- 静态对象自动注册模块 `ek_static_alloc`（`EKCFG_STATIC_ALLOC`，默认关闭）。`EK_DEFINE_*` 把对象登记到链接器段 `.ek_static_alloc`，由 `ek_static_alloc_init()` 按 `order` 升序初始化；`EKCFG_EXPORT=1` 时经 `EK_EXPORT_COMPONENTS` 在 `ek_export_init()` 中自动调用，否则须在堆初始化之后手动调用。失败时记录首个错误（`ek_static_alloc_get_init_error()`）并继续后续对象。
- 静态定义宏与对应 `*_init_static` / `*_deinit_static`：
  - 容器（order=10）：`EK_DEFINE_STACK(handle, type, amount)`、`EK_DEFINE_RINGBUF(handle, type, amount)`、`EK_DEFINE_RINGBUF_SPSC(handle, type, amount)`
  - 调度对象（order=20）：`EK_DEFINE_PT(handle, cb, prio, arg)`、`EK_DEFINE_PT_SEM(handle, count)`、`EK_DEFINE_PT_MSG(handle, type, amount)`、`EK_DEFINE_EVOKE_TASK(handle, cb, arg)`、`EK_DEFINE_EVOKE_EVENT(handle, init_count)`
- TLSF 索引改为配置宏，无需改第三方源码：`EKCFG_TLSF_FL_INDEX_MAX`（默认 24）、`EKCFG_TLSF_SL_INDEX_COUNT_LOG2`（默认 3）。
- 第三方源码按配置空编译：`third_party/tlsf` 由 `EKCFG_HEAP_TLSF==1` 守卫，`third_party/lwprintf` 由 `EKCFG_IO_LWPRTF==1` 守卫。
- 微线程最低优先级可配：`EKCFG_PT_PRIO_LOWEST`（默认 31，就绪队列档位数为该值 + 1）。
- `ek_io.h` 在非 lwprintf/picolibc 路径下补 `#include <stdio.h>`，方便用户把 `ek_printf` 映射到标准 libc。

### 变更

- `ek_pt_msg_t.rb` 从 `ek_ringbuf_t *` 改为内嵌 `ek_ringbuf_t`，静态消息队列不再额外声明独立 ringbuf 对象。
- README / `ek_conf_template.h` / `ek_conf_internal.h` 同步静态分配、TLSF 配置宏与 IO 后端说明。

### 破坏性变更

从 v1.2.0 升级必须改调用点，无兼容别名：

- `ek_evoke_task_create(const char *name, ek_evoke_cb_t cb, void *arg)` → `ek_evoke_task_create(ek_evoke_cb_t cb, void *arg)`
- `ek_evoke_event_create(const char *name, uint32_t init)` → `ek_evoke_event_create(uint32_t init)`
- `ek_pt_create(const char *name, ek_pt_cb_t cb, uint8_t prio, void *arg)` → `ek_pt_create(ek_pt_cb_t cb, uint8_t prio, void *arg)`
- `ek_evoke_task` / `ek_evoke_event` / `ek_pt` 去掉 `const char *name` 字段。
- 直接读写 `ek_pt_msg_t.rb` 指针的代码改为使用内嵌 `ek_ringbuf_t`。
- 启用静态分配时，链接脚本必须增加 `.ek_static_alloc` 段（`KEEP(*(SORT(.ek_static_alloc*)))`，符号 `_ek_static_alloc_start` / `_ek_static_alloc_end`）。

[2.0.0rc]: https://github.com/EmbeddedKitOrg/EK_UTILS/compare/v1.2.0...v2.0.0rc
