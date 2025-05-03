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

time_wheel_t *time_wheel_init(int slot_count, int tick_interval);

// 添加任务到时间轮
void add_task(time_wheel_t *time_wheel, int timeout_seconds, task_callback cb, void *arg);

// 执行当前槽的任务
void exec_slot(time_wheel_t *time_wheel);

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