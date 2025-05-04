#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H
#include "time_wheel.h"
#include "ring_buffer.h"

#define SLOT_COUNT 16
#define TICK_INTERVAL 1
#define SYN_QUEUE_CAPACITY 8192
#define RETRANS_QUEUE_CAPACITY 8192
#define CLOSE_QUEUE_CAPACITY 8192

// 第二种任务：TCP重发
struct retrans_task_t
{
    // 重发不是以client进行的，而是以packet进行的
    // 也是需要一个表，这个表里面存放着全体packet
    int packet_index;

    int retry_times;
};

// 第三种任务：重发断开连接过程中的第二次挥手

class task_queue_t
{
private:
    time_wheel_t *time_wheel = nullptr;

    ring_buffer_t *queue = nullptr;

    // 给std::thread用的
    void exec_handler();

    void put_tasks();
    void tick();

public:
    int task_queue_init();

    int add_task(task_t *task);
    void launch();

    static task_queue_t &get_instance();
};

#endif