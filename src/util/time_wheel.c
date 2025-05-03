#ifndef TIME_WHEEL_H
#define TIME_WHEEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>

#define MAX_EVENTS 10

typedef void (*task_callback)(void *);

typedef struct task_t
{
    int rotation;        // 还需要转几圈
    task_callback cb;    // 回调函数
    void *arg;           // 参数
    struct task_t *next; // 链表
} task_t;

typedef struct
{
    task_t **slots;
    int current_slot;
    int slot_count;
    int tick_interval;
} time_wheel_t;

int time_wheel_init(time_wheel_t *tw, int slot_count, int tick_interval)
{
    tw->slots = (task_t **)malloc(slot_count * sizeof(task_t *));
    if (tw->slots == NULL)
    {
        printf("failed to malloc for time wheel slots\n");
        free(tw);
        tw = NULL;
        return 0;
    }

    for (int i = 0; i < slot_count; i++)
    {
        tw->slots[i] = NULL;
    }
    tw->current_slot = 0;
    tw->slot_count = slot_count;
    tw->tick_interval = tick_interval;

    return 1;
}

// 添加任务到时间轮
void add_task(time_wheel_t *time_wheel, int timeout_seconds, task_callback cb, void *arg)
{
    int ticks = timeout_seconds / time_wheel->tick_interval;
    int rotation = ticks / time_wheel->slot_count;
    int slot = (time_wheel->current_slot + (ticks % time_wheel->slot_count)) % time_wheel->slot_count;

    task_t *task = (task_t *)malloc(sizeof(task_t));
    task->rotation = rotation;
    task->cb = cb;
    task->arg = arg;
    task->next = time_wheel->slots[slot];
    time_wheel->slots[slot] = task;

    printf("Add task to slot %d (rotation=%d)\n", slot, rotation);
}

// 执行当前槽的任务
void exec_slot(time_wheel_t *time_wheel)
{
    task_t **pp = &time_wheel->slots[time_wheel->current_slot];
    while (*pp)
    {
        task_t *t = *pp;
        if (t->rotation > 0)
        {
            t->rotation--;
            pp = &t->next;
        }
        else
        {
            // 执行任务
            t->cb(t->arg);
            *pp = t->next;
            free(t);
        }
    }

    time_wheel->current_slot = (time_wheel->current_slot + 1) % time_wheel->slot_count;
}

// int main()
// {
//     int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
//     struct itimerspec new_value = {
//         .it_value.tv_sec = TICK_INTERVAL,
//         .it_interval.tv_sec = TICK_INTERVAL,
//     };
//     timerfd_settime(tfd, 0, &new_value, NULL);

//     int epfd = epoll_create1(0);
//     struct epoll_event ev = {
//         .events = EPOLLIN,
//         .data.fd = tfd,
//     };
//     epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

//     while (1)
//     {
//         struct epoll_event events[MAX_EVENTS];
//         int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
//         for (int i = 0; i < nfds; ++i)
//         {
//             if (events[i].data.fd == tfd)
//             {
//                 uint64_t expirations;
//                 read(tfd, &expirations, sizeof(expirations));
//                 for (uint64_t j = 0; j < expirations; ++j)
//                     tick();
//             }
//         }
//     }

//     close(tfd);
//     close(epfd);
//     return 0;
// }

#endif