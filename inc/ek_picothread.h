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

typedef struct ek_pt_sem_t ek_pt_sem_t;

struct ek_pt_sem_t
{
    uint8_t count;
    ek_list_node_t wait_list;
};

#    ifdef __cplusplus
extern "C"
{
#    endif /* __cplusplus */

#    ifdef __cplusplus
}
#    endif /* __cplusplus */

#endif /* EKCFG_PICOTHREAD */

#endif /* EK_PICOTHREAD_H */
