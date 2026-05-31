#include "ek_picothread.h"

#if EKCFG_PICOTHREAD == 1

#    include "ek_heap.h"
#    include "ek_assert.h"
#    include "ek_export.h"

__EK_STATIC_INLINE void _insert_state_by_prio(ek_list_node_t *list, ek_pt_t *pt);
__EK_STATIC_INLINE void _insert_event_by_prio(ek_list_node_t *list, ek_pt_t *pt);
__EK_STATIC_INLINE void _insert_state_by_time(ek_list_node_t *list, ek_pt_t *pt);
static ek_pt_t *_pt_next_ready(void);

volatile static bool s_init = false;
static ek_list_node_t s_ready_list; // 就绪链表
static ek_list_node_t s_block_list; // 阻塞链表
static ek_pt_t *s_cur_pt; // 当前正在运行的线程
static ek_pt_t *s_next_pt; // 下一个就绪的任务
static uint32_t s_schedule_now; // 当前调度基准 tick

void ek_pt_init(void)
{
    if (s_init == true)
    {
        return;
    }
    s_init = true;
    ek_list_init(&s_ready_list);
    ek_list_init(&s_block_list);
}

EK_EXPORT_COMPONENTS(ek_pt_init);

ek_pt_handle_t ek_pt_create(const char *name, ek_pt_cb_t cb, uint8_t prio, void *arg)
{
    ek_assert_param(s_init == true);
    ek_assert_param(cb != NULL);
    ek_pt_t *pt = ek_malloc(sizeof(*pt));
    ek_assert_param(pt != NULL);
    pt->name = name;
    pt->cb = cb;
    pt->arg = arg;
    pt->prio = prio;
    pt->state = EK_PT_STATE_READY;
    pt->tick = 0;
    pt->line = 0;
    ek_list_init(&pt->event_node);
    ek_list_init(&pt->state_node);

    _insert_state_by_prio(&s_ready_list, pt);

    return pt;
}

void ek_pt_destroy(ek_pt_t *pt)
{
    ek_assert_param(pt != NULL);
    // TODO 在删除任务的时候需要考虑在事件节点中的相关处理
}

uint32_t ek_pt_schedule(uint32_t now)
{
    ek_assert_param(s_init == true);

    s_schedule_now = now;

    // 将阻塞链表中到期任务移回就绪链表
    ek_list_node_t *pos, *n;
    ek_list_foreach_safe(pos, n, &s_block_list)
    {
        ek_pt_t *pt = ek_list_container(pos, ek_pt_t, state_node);
        if (pt->tick > now)
        {
            break;
        }
        ek_list_remove(pos);
        pt->state = EK_PT_STATE_READY;
        _insert_state_by_prio(&s_ready_list, pt);
    }

    // 取下一个就绪任务
    s_next_pt = _pt_next_ready();
    if (s_next_pt == NULL)
    {
        // 无就绪任务，跳过执行
        s_cur_pt = NULL;
    }
    else
    {
        s_cur_pt = s_next_pt;
        ek_list_remove(&s_cur_pt->state_node);
        s_cur_pt->state = EK_PT_STATE_RUNNING;
        s_cur_pt->cb(s_cur_pt, s_cur_pt->arg);

        // 回调后根据状态重新分流
        if (s_cur_pt->state == EK_PT_STATE_RUNNING)
        {
            s_cur_pt->state = EK_PT_STATE_READY;
            _insert_state_by_prio(&s_ready_list, s_cur_pt);
        }
        else if (s_cur_pt->state == EK_PT_STATE_BLOCK)
        {
            _insert_state_by_time(&s_block_list, s_cur_pt);
        }
    }

    // 有就绪任务则返回当前 tick，主循环不应睡眠
    if (!ek_list_is_empty(&s_ready_list)) return s_schedule_now;

    // 返回阻塞链表最早唤醒 tick
    // 空则 0 表示可无限休眠
    if (ek_list_is_empty(&s_block_list)) return 0;
    ek_list_node_t *head = ek_list_get_first(&s_block_list);
    ek_pt_t *first_blocked = ek_list_container(head, ek_pt_t, state_node);
    return first_blocked->tick;
}

void ek_pt_block(ek_pt_handle_t pt, uint32_t xtick)
{
    ek_assert_param(s_init == true);
    ek_assert_param(pt != NULL);

    pt->tick = s_schedule_now + xtick;
    if (pt == s_cur_pt)
    {
        pt->state = EK_PT_STATE_BLOCK;
        return;
    }

    if (pt->state == EK_PT_STATE_READY || pt->state == EK_PT_STATE_BLOCK)
    {
        ek_list_remove(&pt->state_node);
    }
    pt->state = EK_PT_STATE_BLOCK;
    _insert_state_by_time(&s_block_list, pt);
}

ek_pt_handle_t ek_pt_suspend(ek_pt_handle_t pt)
{
    // 传入 NULL 表示挂起自己
    ek_pt_t *pt_to_suspend = pt == NULL ? s_cur_pt : pt;

    ek_assert_param(s_init == true);
    ek_assert_param(pt_to_suspend != NULL);

    if (pt_to_suspend == s_cur_pt)
    {
        pt_to_suspend->state = EK_PT_STATE_SUSPEND;
        return pt_to_suspend;
    }

    if (pt_to_suspend->state == EK_PT_STATE_READY || pt_to_suspend->state == EK_PT_STATE_BLOCK)
    {
        ek_list_remove(&pt_to_suspend->state_node);
    }
    pt_to_suspend->state = EK_PT_STATE_SUSPEND;
    return NULL;
}

void ek_pt_resume(ek_pt_handle_t pt)
{
    ek_assert_param(s_init == true);
    ek_assert_param(pt != NULL);
    ek_assert_param(pt->state == EK_PT_STATE_SUSPEND);

    pt->state = EK_PT_STATE_READY;
    _insert_state_by_prio(&s_ready_list, pt);
}

#    if EKCFG_PICOTHREAD_SEM == 1
ek_pt_sem_t *ek_pt_sem_create(uint8_t count)
{
    ek_pt_sem_t *sem = ek_malloc(sizeof(*sem));
    ek_assert_param(sem != NULL);
    sem->count = count;
    ek_list_init(&sem->wait_list);
    return sem;
}

void ek_pt_sem_destroy(ek_pt_sem_t *sem)
{
    ek_assert_param(sem != NULL);
    // TODO 在删除任务的时候需要考虑在事件节点中的相关处理
}
#    endif /* EKCFG_PICOTHREAD_SEM */

#    if EKCFG_PICOTHREAD_MSG == 1
// TODO 消息队列相关
#    endif /* EKCFG_PICOTHREAD_MSG */

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
            return;
        }
    }
    ek_list_insert_tail(list, &pt->state_node);
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
            return;
        }
    }
    ek_list_insert_tail(list, &pt->event_node);
}

__EK_STATIC_INLINE void _insert_state_by_time(ek_list_node_t *list, ek_pt_t *pt)
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
        if (cur_pt->tick > pt->tick || (cur_pt->tick == pt->tick && cur_pt->prio > pt->prio))
        {
            ek_list_insert_before(i, &pt->state_node);
            return;
        }
    }
    ek_list_insert_tail(list, &pt->state_node);
}

static ek_pt_t *_pt_next_ready(void)
{
    if (ek_list_is_empty(&s_ready_list))
    {
        return NULL;
    }
    ek_list_node_t *head = ek_list_get_first(&s_ready_list);
    return ek_list_container(head, ek_pt_t, state_node);
}

#endif // EKCFG_PICOTHREAD
