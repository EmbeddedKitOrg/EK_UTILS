#ifndef EK_STATIC_ALLOC_H
#define EK_STATIC_ALLOC_H

#include "ek_conf_internal.h"

#if EKCFG_STATIC_ALLOC == 1

#    include "ek_def.h"
#    include "ek_err.h"

#    ifdef __cplusplus
extern "C"
{
#    endif

typedef ek_err_t (*_ek_static_alloc_init_fn_t)(void);

typedef struct
{
    uint16_t order;
    const char *name;
    _ek_static_alloc_init_fn_t fn;
} _ek_static_alloc_item_t;

#    define _EK_STATIC_ALLOC_JOIN2(a, b) a##b
#    define _EK_STATIC_ALLOC_JOIN(a, b)  _EK_STATIC_ALLOC_JOIN2(a, b)
#    define _EK_STATIC_ALLOC_STRING2(x)  #x
#    define _EK_STATIC_ALLOC_STRING(x)   _EK_STATIC_ALLOC_STRING2(x)

#    define EK_STATIC_ALLOC_REGISTER(instance, order, fn)                                             \
        static const _ek_static_alloc_item_t _EK_STATIC_ALLOC_JOIN(__ek_static_alloc_item_, instance) \
        __EK_USED __EK_SECTION(".ek_static_alloc") = { (uint16_t)(order), _EK_STATIC_ALLOC_STRING(instance), (fn) }

void ek_static_alloc_init(void);
ek_err_t ek_static_alloc_get_init_error(void);

#    ifdef __cplusplus
}
#    endif

#endif /* EKCFG_STATIC_ALLOC == 1 */

#endif /* EK_STATIC_ALLOC_H */
