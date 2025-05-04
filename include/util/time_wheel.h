#pragma once
#include <stdint.h>
#include <vector>

#define MAX_EVENTS 10

typedef void (*task_callback)(void *);

class task_t
{
    // time秒后执行
public:
    virtual ~task_t() {}
    int time;
    int rotation;

    // 可以重载
    virtual void handle() = 0;
};

class time_wheel_t
{
    std::vector<std::vector<task_t *>> slots;
    int current_slot;
    int slot_count;
    int tick_interval;

public:
    int time_wheel_init(size_t slot_count, int tick_interval);

    // 添加任务到时间轮
    void add_task(task_t *task);

    // 执行当前槽的任务，然后current++
    void exec_slot();
};
