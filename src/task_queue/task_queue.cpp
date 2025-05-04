#include "task_queue.h"
#include "syn_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <thread>

void task_queue_t::exec_handler()
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec new_value;
    new_value.it_value.tv_sec = TICK_INTERVAL;
    new_value.it_interval.tv_sec = TICK_INTERVAL;
    timerfd_settime(tfd, 0, &new_value, nullptr);

    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    while (1)
    {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == tfd)
            {
                uint64_t expirations;
                read(tfd, &expirations, sizeof(expirations));
                for (uint64_t j = 0; j < expirations; ++j)
                    tick();
            }
        }
    }

    close(tfd);
    close(epfd);
}

void task_queue_t::put_tasks()
{
    auto handle_syn_task = [this](void *p)
    {
        task_t *task = *(task_t **)p;
        this->time_wheel->add_task(task);
    };
    this->queue->handle(handle_syn_task);
}

void task_queue_t::tick()
{
    // 先把队列里面的加入到时间轮的相应位置，然后再正式开始处理
    this->put_tasks();

    this->time_wheel->exec_slot();
}

int task_queue_t::task_queue_init()
{
    int result = 0;
    time_wheel = new time_wheel_t;

    result = time_wheel->time_wheel_init(SLOT_COUNT, TICK_INTERVAL);
    if (result != 1)
    {
        printf("failed to init time_wheel\n");
        return -1;
    }

    queue = new ring_buffer_t;
    result = queue->init(8192 * 4, sizeof(task_t *));
    if (result != 1)
    {
        printf("failed to init ring buffer for syn queue\n");
        return result;
    }

    return 1;
}

task_queue_t &task_queue_t::get_instance()
{
    static task_queue_t task_queue;

    return task_queue;
}

// 把任务先暂存在这个队列里面，等时间轮tick的时候再处理
int task_queue_t::add_task(task_t *task)
{
    int result = this->queue->push(&task);
    if (result != 1)
    {
        printf("failed to add task\n");
        return result;
    }

    return 1;
}

void task_queue_t::launch()
{
    std::thread t([this]() {
        this->exec_handler();
    });
    t.detach();
    printf("task queue thread started...\n");
}