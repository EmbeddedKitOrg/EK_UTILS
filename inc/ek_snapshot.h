#ifndef EK_SNAPSHOT_H
#define EK_SNAPSHOT_H

#include "ek_conf_internal.h"

#if EKCFG_SNAPSHOT == 1

#    include "ek_err.h"
#    include "ek_def.h"

typedef struct ek_snapshot ek_snapshot_t;

struct ek_snapshot
{
    uint32_t unique; /**< 当前快照的唯一id，可以是时间戳一类 */
#    if EKCFG_RTOS == 1
    EK_LOCK_TYPE lock;
#    endif /* EKCFG_RTOS */
};

#    ifdef __cplusplus
extern "C"
{
#    endif /* __cplusplus */

#    ifdef __cplusplus
}
#    endif /* __cplusplus */

#endif // EKCFG_SNAPSHOT == 1

#endif // EK_SNAPSHOT_H
