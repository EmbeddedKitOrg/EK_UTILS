#ifndef EK_PICOTHREAD_H
#define EK_PICOTHREAD_H

#include "ek_conf_internal.h"

#if EKCFG_PICOTHREAD == 1

#    include "ek_err.h"
#    include "ek_list.h"

typedef struct ek_pt_t ek_pt_t;
typedef ek_pt_t *ek_pt_handle_t;
typedef void (*ek_pt_cb_t)(ek_pt_handle_t pt, void *arg);

typedef enum
{
    EK_PT_STATE_READY = 0,
    EK_PT_STATE_SUSPEND,
    EK_PT_STATE_BLOCK,
    EK_PT_STATE_RUNNING,

    EK_PT_STATE_MAX
} ek_pt_state_t;

struct ek_pt_t
{
    const char *name;
    uint8_t prio;
    ek_pt_state_t state;
    uint32_t tick;
    uint32_t line;
    ek_list_node_t state_node;
    ek_list_node_t event_node;
    void *arg;
    ek_pt_cb_t cb;
};

#    if EKCFG_PICOTHREAD_SEM == 1
typedef struct ek_pt_sem_t ek_pt_sem_t;
typedef ek_pt_sem_t *ek_pt_sem_handle_t;

struct ek_pt_sem_t
{
    uint8_t count;
    ek_list_node_t wait_list;
};
#    endif /* EKCFG_PICOTHREAD_SEM */

#    if EKCFG_PICOTHREAD_MSG == 1
// TODO 消息队列相关数据结构
#    endif /* EKCFG_PICOTHREAD_MSG */

#    define EK_PT_BEGIN(pt) \
        switch ((pt)->line) \
        {                   \
            case 0:

#    define EK_PT_YEILD(pt)    \
        (pt)->line = __LINE__; \
        return;                \
        case __LINE__:

#    define EK_PT_END(pt) \
        (pt)->line = 0;   \
        }

#    ifdef __cplusplus
extern "C"
{
#    endif /* __cplusplus */

void ek_pt_init(void);
ek_pt_handle_t ek_pt_create(const char *name, ek_pt_cb_t cb, uint8_t prio, void *arg);
void ek_pt_destroy(ek_pt_handle_t pt);
uint32_t ek_pt_schedule(uint32_t now);
void ek_pt_block(ek_pt_handle_t pt, uint32_t xtick);
ek_pt_t *ek_pt_suspend(ek_pt_handle_t pt);
void ek_pt_resume(ek_pt_handle_t pt);

#    define EK_PT_DELAY(pt, xtick)      \
        do                              \
        {                               \
            ek_pt_block((pt), (xtick)); \
            EK_PT_YEILD(pt);            \
        } while (0)

#    define EK_PT_SUSPEND(pt)         \
        do                            \
        {                             \
            ek_pt_t *_spt;            \
            _spt = ek_pt_suspend(pt); \
            if (_spt != NULL)         \
            {                         \
                EK_PT_YEILD(_spt);    \
            }                         \
        } while (0)

#    define EK_PT_RESUME(pt) ek_pt_resume(pt)

#    if EKCFG_PICOTHREAD_SEM == 1
// TODO 信号量的函数声明
#    endif /* EKCFG_PICOTHREAD_SEM */

#    if EKCFG_PICOTHREAD_MSG == 1
// TODO 消息队列的函数声明
#    endif /* EKCFG_PICOTHREAD_MSG */

#    ifdef __cplusplus
}
#    endif /* __cplusplus */

#endif /* EKCFG_PICOTHREAD */

#endif /* EK_PICOTHREAD_H */
