#ifndef TIME_WHEEL_H
#define TIME_WHEEL_H
#include "time_wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>

#define MAX_EVENTS 10

int time_wheel_t::time_wheel_init(int slot_count, int tick_interval)
{
    this->slots = (task_t **)malloc(slot_count * sizeof(task_t *));
    if (this->slots == NULL)
    {
        printf("failed to malloc for time wheel slots\n");
        return 0;
    }

    for (int i = 0; i < slot_count; i++)
    {
        this->slots[i] = NULL;
    }
    this->current_slot = 0;
    this->slot_count = slot_count;
    this->tick_interval = tick_interval;

    return 1;
}

// 添加任务到时间轮
void time_wheel_t::add_task(int timeout_seconds, task_callback cb, void *arg)
{
    int ticks = timeout_seconds / this->tick_interval;
    int rotation = ticks / this->slot_count;
    int slot = (this->current_slot + (ticks % this->slot_count)) % this->slot_count;

    task_t *task = (task_t *)malloc(sizeof(task_t));
    task->rotation = rotation;
    task->cb = cb;
    task->arg = arg;
    task->next = this->slots[slot];
    this->slots[slot] = task;

    printf("Add task to slot %d (rotation=%d)\n", slot, rotation);
}

// 执行当前槽的任务
void time_wheel_t::exec_slot()
{
    task_t **pp = &slots[this->current_slot];
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

    this->current_slot = (this->current_slot + 1) % this->slot_count;
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