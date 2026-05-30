#ifndef EK_PICOTHREAD_H
#define EK_PICOTHREAD_H

#include "ek_conf_internal.h"

#if EKCFG_PICOTHREAD == 1

#    include "ek_err.h"
#    include "ek_list.h"

typedef struct ek_pt_t ek_pt_t;
typedef void (*ek_pt_cb_t)(ek_pt_t *pt, void *arg);

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

#    define EK_PT_BEGIN(ptr_pt) \
        switch ((ptr_pt)->line) \
        {                       \
            case 0:

#    define EK_PT_YEILD(ptr_pt)        \
        do                             \
        {                              \
            (ptr_pt)->line = __LINE__; \
            return;                    \
            case __LINE__:             \
        } while (0)

#    define EK_PT_END(ptr_pt) \
        (ptr_pt)->line = 0;   \
        }

#    define EK_PT_DELAY(ptr_pt, xtick)      \
        do                                  \
        {                                   \
            ek_pt_block((ptr_pt), (xtick)); \
            EK_PT_YEILD(ptr_pt);            \
        } while (0)

#    define EK_PT_SUSPEND(ptr_pt)         \
        do                                \
        {                                 \
            ek_pt_t *_spt;                \
            _spt = ek_pt_suspend(ptr_pt); \
            if (_spt != NULL)             \
            {                             \
                EK_PT_YEILD(_spt);        \
            }                             \
        } while (0)

#    define EK_PT_RESUME(ptr_pt) ek_pt_resume(ptr_pt)

#    if EKCFG_PICOTHREAD_SEM == 1
typedef struct ek_pt_sem_t ek_pt_sem_t;

struct ek_pt_sem_t
{
    uint8_t count;
    ek_list_node_t wait_list;
};
#    endif /* EKCFG_PICOTHREAD_SEM */

#    if EKCFG_PICOTHREAD_MSG == 1
// TODO 消息队列相关

#    endif /* EKCFG_PICOTHREAD_MSG */

#    ifdef __cplusplus
extern "C"
{
#    endif /* __cplusplus */
__EK_WEAK void ek_pt_idle_hook(void);

void ek_pt_init(void);
ek_pt_t *ek_pt_create(const char *name, ek_pt_cb_t cb, uint8_t prio, void *arg);
void ek_pt_destroy(ek_pt_t *pt);
uint32_t ek_pt_schedule(uint32_t now);
void ek_pt_block(ek_pt_t *pt, uint32_t xtick);
ek_pt_t *ek_pt_suspend(ek_pt_t *pt);
void ek_pt_resume(ek_pt_t *pt);

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
