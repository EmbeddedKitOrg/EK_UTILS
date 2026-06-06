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

EK_EXPORT_COMPONENTS(ek_pt_init, 0);

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
#    if EKCFG_PICOTHREAD_SEM || EKCFG_PICOTHREAD_MSG
    pt->wait_result = EK_ERR_NONE;
#    endif
#    if EKCFG_PICOTHREAD_SEM
    pt->wait_sem = NULL;
#    endif /* EKCFG_PICOTHREAD_SEM */
#    if EKCFG_PICOTHREAD_MSG
    pt->wait_msg = NULL;
#    endif /* EKCFG_PICOTHREAD_MSG */

    _insert_state_by_prio(&s_ready_list, pt);

    return pt;
}

void ek_pt_destroy(ek_pt_handle_t pt)
{
    ek_assert_param(pt != NULL);
    ek_assert_param(pt != s_cur_pt);

    // 从状态链表移除（就绪或阻塞）
    if (pt->state == EK_PT_STATE_READY || pt->state == EK_PT_STATE_BLOCK)
    {
        ek_list_remove(&pt->state_node);
    }

#    if EKCFG_PICOTHREAD_SEM
    // 从信号量等待链表移除
    if (pt->wait_sem != NULL)
    {
        ek_list_remove(&pt->event_node);
        pt->wait_sem = NULL;
    }
#    endif
#    if EKCFG_PICOTHREAD_MSG
    // 从消息队列等待链表移除
    if (pt->wait_msg != NULL)
    {
        ek_list_remove(&pt->event_node);
        pt->wait_msg = NULL;
    }
#    endif
#    if EKCFG_PICOTHREAD_SEM || EKCFG_PICOTHREAD_MSG
    pt->wait_result = EK_ERR_ABORTED;
#    endif

    ek_free(pt);
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
#    if EKCFG_PICOTHREAD_SEM || EKCFG_PICOTHREAD_MSG
        // 超时退出时清理事件等待链表
#        if EKCFG_PICOTHREAD_SEM
        if (pt->wait_sem != NULL)
        {
            ek_list_remove(&pt->event_node);
            pt->wait_sem = NULL;
            pt->wait_result = EK_ERR_TIMEOUT;
        }
#        endif
#        if EKCFG_PICOTHREAD_MSG
        if (pt->wait_msg != NULL)
        {
            ek_list_remove(&pt->event_node);
            pt->wait_msg = NULL;
            pt->wait_result = EK_ERR_TIMEOUT;
        }
#        endif
#    endif
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
    pt->tick = (xtick == (uint32_t)-1) ? (uint32_t)-1 : s_schedule_now + xtick;
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

ek_pt_handle_t ek_pt_active(void)
{
    return s_cur_pt;
}

#    if EKCFG_PICOTHREAD_SEM == 1
ek_pt_sem_handle_t ek_pt_sem_create(uint8_t count)
{
    ek_pt_sem_t *sem = ek_malloc(sizeof(*sem));
    ek_assert_param(sem != NULL);
    sem->count = count;
    ek_list_init(&sem->wait_list);
    return sem;
}

void ek_pt_sem_destroy(ek_pt_sem_handle_t sem)
{
    ek_assert_param(sem != NULL);

    // 唤醒所有等待此信号量的任务
    ek_list_node_t *pos, *n;
    ek_list_foreach_safe(pos, n, &sem->wait_list)
    {
        ek_pt_t *pt = ek_list_container(pos, ek_pt_t, event_node);
        ek_list_remove(pos);
        ek_list_remove(&pt->state_node);
        pt->wait_sem = NULL;
        pt->wait_result = EK_ERR_ABORTED;
        _insert_state_by_prio(&s_ready_list, pt);
        pt->state = EK_PT_STATE_READY;
    }

    ek_free(sem);
}

bool ek_pt_sem_take(ek_pt_sem_handle_t sem)
{
    ek_assert_param(sem != NULL);
    ek_assert_param(s_cur_pt != NULL);

    if (sem->count)
    {
        sem->count--;
        return true;
    }
    s_cur_pt->wait_sem = sem;
    _insert_event_by_prio(&sem->wait_list, s_cur_pt);
    return false;
}

void ek_pt_sem_give(ek_pt_sem_handle_t sem)
{
    ek_assert_param(sem != NULL);

    // 首先判断是否有等待的任务
    // 如果有，则取出优先级最高
    // 删除事件节点的所在链表位置
    // 设置为就绪状态
    if (!ek_list_is_empty(&sem->wait_list))
    {
        ek_list_node_t *node = ek_list_get_first(&sem->wait_list);
        ek_pt_t *pt = ek_list_container(node, ek_pt_t, event_node);
        ek_list_remove(node);
        ek_list_remove(&pt->state_node);
        pt->wait_sem = NULL;
        pt->wait_result = EK_ERR_NONE;
        _insert_state_by_prio(&s_ready_list, pt);
        pt->state = EK_PT_STATE_READY;
    }

    // 如果没有，则增加计数值
    sem->count++;
}

#    endif /* EKCFG_PICOTHREAD_SEM */

#    if EKCFG_PICOTHREAD_MSG == 1
#        include "ek_ringbuf.h"

ek_pt_msg_handle_t ek_pt_msg_create(size_t item_size, uint32_t item_amount)
{
    ek_pt_msg_t *msg = ek_malloc(sizeof(*msg));
    ek_assert_param(msg != NULL);

    msg->rb = ek_ringbuf_create(item_size, item_amount);
    if (msg->rb == NULL)
    {
        ek_free(msg);
        return NULL;
    }

    ek_list_init(&msg->recv_wait);
    ek_list_init(&msg->send_wait);
    return msg;
}

void ek_pt_msg_destroy(ek_pt_msg_handle_t msg)
{
    ek_assert_param(msg != NULL);

    // 唤醒所有等待接收的任务
    ek_list_node_t *pos, *n;
    ek_list_foreach_safe(pos, n, &msg->recv_wait)
    {
        ek_pt_t *pt = ek_list_container(pos, ek_pt_t, event_node);
        ek_list_remove(pos);
        ek_list_remove(&pt->state_node);
        pt->wait_msg = NULL;
        pt->wait_result = EK_ERR_ABORTED;
        _insert_state_by_prio(&s_ready_list, pt);
        pt->state = EK_PT_STATE_READY;
    }

    // 唤醒所有等待发送的任务
    ek_list_foreach_safe(pos, n, &msg->send_wait)
    {
        ek_pt_t *pt = ek_list_container(pos, ek_pt_t, event_node);
        ek_list_remove(pos);
        ek_list_remove(&pt->state_node);
        pt->wait_msg = NULL;
        pt->wait_result = EK_ERR_ABORTED;
        _insert_state_by_prio(&s_ready_list, pt);
        pt->state = EK_PT_STATE_READY;
    }

    ek_ringbuf_destroy(msg->rb);
    ek_free(msg);
}

bool ek_pt_msg_send(ek_pt_msg_handle_t msg, const void *data)
{
    ek_assert_param(msg != NULL);
    ek_assert_param(data != NULL);
    ek_assert_param(s_cur_pt != NULL);

    ek_err_t ret = ek_ringbuf_write(msg->rb, data);
    if (ret == EK_ERR_NONE)
    {
        // 写入成功，唤醒一个等待接收的任务
        if (!ek_list_is_empty(&msg->recv_wait))
        {
            ek_list_node_t *node = ek_list_get_first(&msg->recv_wait);
            ek_pt_t *pt = ek_list_container(node, ek_pt_t, event_node);
            ek_list_remove(node);
            ek_list_remove(&pt->state_node);
            pt->wait_msg = NULL;
            pt->wait_result = EK_ERR_NONE;
            _insert_state_by_prio(&s_ready_list, pt);
            pt->state = EK_PT_STATE_READY;
        }
        return true;
    }

    // 缓冲区满，挂到发送等待队列
    s_cur_pt->wait_msg = msg;
    _insert_event_by_prio(&msg->send_wait, s_cur_pt);
    return false;
}

bool ek_pt_msg_recv(ek_pt_msg_handle_t msg, void *data)
{
    ek_assert_param(msg != NULL);
    ek_assert_param(s_cur_pt != NULL);

    ek_err_t ret = ek_ringbuf_read(msg->rb, data);
    if (ret == EK_ERR_NONE)
    {
        // 读取成功，唤醒一个等待发送的任务
        if (!ek_list_is_empty(&msg->send_wait))
        {
            ek_list_node_t *node = ek_list_get_first(&msg->send_wait);
            ek_pt_t *pt = ek_list_container(node, ek_pt_t, event_node);
            ek_list_remove(node);
            ek_list_remove(&pt->state_node);
            pt->wait_msg = NULL;
            pt->wait_result = EK_ERR_NONE;
            _insert_state_by_prio(&s_ready_list, pt);
            pt->state = EK_PT_STATE_READY;
        }
        return true;
    }

    // 缓冲区空，挂到接收等待队列
    s_cur_pt->wait_msg = msg;
    _insert_event_by_prio(&msg->recv_wait, s_cur_pt);
    return false;
}

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
