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

int time_wheel_t::time_wheel_init(size_t slot_count, int tick_interval)
{
    this->slots = std::vector<std::vector<task_t *>>(slot_count);
    for (size_t i = 0; i < slot_count; i++)
    {
        this->slots[i] = std::vector<task_t *>(0);
    }
    this->current_slot = 0;
    this->slot_count = slot_count;
    this->tick_interval = tick_interval;

    return 1;
}

// 添加任务到时间轮
void time_wheel_t::add_task(task_t *task)
{
    int ticks = task->time / this->tick_interval;
    int rotation = ticks / this->slot_count;
    int slot = (this->current_slot + (ticks % this->slot_count)) % this->slot_count;

    task->rotation = rotation;
    this->slots[slot].push_back(task);

    printf("Add task to slot %d (rotation=%d)\n", slot, rotation);
}

void time_wheel_t::exec_slot()
{
    std::vector<task_t *> &slot = slots[this->current_slot];
    for (auto it = slot.begin(); it != slot.end(); ) {
        if ((*it)->rotation > 0)
        {
            (*it)->rotation--;
        }
        else {
            (*it)->handle();
            delete (*it);
            it = slot.erase(it);
            continue;
        }
        ++it;
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
//     timerfd_settime(tfd, 0, &new_value, nullptr);

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
