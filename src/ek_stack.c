/**
 * @file ek_stack.c
 * @brief 栈数据结构实现
 * @author N1netyNine99
 */

#include "ek_stack.h"

#if EKCFG_STACK == 1

#    include "ek_assert.h"
#    include "ek_heap.h"

bool ek_stack_full(ek_stack_t *sk)
{
    ek_assert_param(sk != NULL);
    return sk->sp >= sk->cap;
}

bool ek_stack_empty(ek_stack_t *sk)
{
    ek_assert_param(sk != NULL);
    return sk->sp == 0;
}

ek_stack_t *ek_stack_create(size_t item_size, uint32_t item_amount)
{
    ek_assert_param(item_amount != 0);
    ek_assert_param(item_size != 0);

    ek_stack_t *sk = (ek_stack_t *)ek_malloc(sizeof(ek_stack_t));
    if (sk == NULL)
    {
        return NULL;
    }
    sk->buffer = (uint8_t *)ek_malloc(item_amount * item_size);
    if (sk->buffer == NULL)
    {
        ek_free(sk);
        return NULL;
    }

    sk->sp = 0;
    sk->cap = item_amount;
    sk->item_size = item_size;
#    if EKCFG_RTOS == 1
    EK_LOCK_INIT(sk->lock);
#    endif /* EKCFG_RTOS */

    return sk;
}

void ek_stack_destroy(ek_stack_t *sk)
{
    ek_assert_param(sk != NULL);

#    if EKCFG_RTOS == 1
    EK_LOCK_DEINIT(sk->lock);
#    endif
    ek_free(sk->buffer);
    ek_free(sk);
}

ek_err_t ek_stack_push(ek_stack_t *sk, const void *item)
{
    ek_assert_param(sk != NULL);
    ek_assert_param(item != NULL);

    if (!EK_LOCK_TRY(sk)) return EK_ERR_BUSY;

    if (ek_stack_full(sk) == true)
    {
        EK_LOCK_RELEASE(sk);
        return EK_ERR_FULL;
    }

    uint8_t *target = (uint8_t *)sk->buffer + sk->sp * sk->item_size;
    memcpy(target, item, sk->item_size);
    sk->sp++;

    EK_LOCK_RELEASE(sk);

    return EK_ERR_NONE;
}

ek_err_t ek_stack_pop(ek_stack_t *sk, void *item)
{
    ek_assert_param(sk != NULL);
    ek_assert_param(item != NULL);

    if (!EK_LOCK_TRY(sk)) return EK_ERR_BUSY;

    if (ek_stack_empty(sk) == true)
    {
        EK_LOCK_RELEASE(sk);
        return EK_ERR_EMPTY;
    }

    sk->sp--;
    uint8_t *source = (uint8_t *)sk->buffer + sk->sp * sk->item_size;
    memcpy(item, source, sk->item_size);

    EK_LOCK_RELEASE(sk);

    return EK_ERR_NONE;
}

#endif /* EKCFG_STACK */
