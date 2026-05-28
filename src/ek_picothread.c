#include "ek_picothread.h"

#if EKCFG_PICOTHREAD == 1

#    include "ek_heap.h"
#    include "ek_assert.h"

__EK_STATIC_INLINE void _insert_state_by_prio(ek_list_node_t *list, ek_pt_t *pt);
__EK_STATIC_INLINE void _insert_event_by_prio(ek_list_node_t *list, ek_pt_t *pt);

volatile static bool s_init = false;
static ek_list_node_t s_ready_list; // 就绪链表
static ek_list_node_t s_block_list; // 阻塞链表
static ek_pt_t *s_cur_pt; // 当前正在运行的线程

ek_pt_t *ek_pt_create(const char *name, ek_pt_cb_t cb, uint8_t prio, void *arg)
{
    ek_assert_param(cb != NULL);
    ek_pt_t *pt = ek_malloc(sizeof(*pt));
    ek_assert_param(pt != NULL);
    pt->name = name;
    pt->cb = cb;
    pt->arg = arg;
    pt->prio = prio;
    pt->state = EK_PT_STATE_READY;
    ek_list_init(&pt->event_node);
    ek_list_init(&pt->state_node);

    if (s_init == false)
    {
        s_init = true;
        ek_list_init(&s_ready_list);
        ek_list_init(&s_block_list);
    }

    _insert_state_by_prio(&s_ready_list, pt);

    return pt;
}

void ek_pt_destory(ek_pt_t *pt)
{
    ek_assert_param(pt != NULL);
    // TODO 在删除任务的时候需要考虑在事件节点中的相关处理
}

ek_err_t ek_pt_suspend(ek_pt_t *pt)
{
    ek_pt_t *pt_to_suspend = pt == NULL ? s_cur_pt : pt;
    ek_assert_param(pt_to_suspend != NULL);
}

ek_pt_sem_t *ek_pt_sem_create(uint8_t count)
{
    ek_pt_sem_t *sem = ek_malloc(sizeof(*sem));
    ek_assert_param(sem != NULL);
    sem->count = count;
    ek_list_init(&sem->wait_list);
    return sem;
}

void ek_pt_sem_destory(ek_pt_sem_t *sem)
{
    ek_assert_param(sem != NULL);
    // TODO 在删除任务的时候需要考虑在事件节点中的相关处理
}

__EK_STATIC_INLINE void _insert_state_by_prio(ek_list_node_t *list, ek_pt_t *pt)
{
    if (ek_list_is_empty(list))
    {
        ek_list_insert_head(list, &pt->state_node);
        return;
    }
    ek_list_node_t *i;
    ek_list_foreach(i, list)
    {
        ek_pt_t *cur_pt = ek_list_container(i, ek_pt_t, state_node);
        if (cur_pt->prio > pt->prio)
        {
            ek_list_insert_before(i, &pt->state_node);
        }
    }
}

__EK_STATIC_INLINE void _insert_event_by_prio(ek_list_node_t *list, ek_pt_t *pt)
{
    if (ek_list_is_empty(list))
    {
        ek_list_insert_head(list, &pt->event_node);
        return;
    }
    ek_list_node_t *i;
    ek_list_foreach(i, list)
    {
        ek_pt_t *cur_pt = ek_list_container(i, ek_pt_t, event_node);
        if (cur_pt->prio > pt->prio)
        {
            ek_list_insert_before(i, &pt->event_node);
        }
    }
}

#endif // EKCFG_PICOTHREAD
