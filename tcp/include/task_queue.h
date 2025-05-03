#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H
#include "time_wheel.h"
#include "ring_buffer.h"

#define SLOT_COUNT 16
#define TICK_INTERVAL 1
#define SYN_QUEUE_CAPACITY 8192
#define RETRANS_QUEUE_CAPACITY 8192
#define CLOSE_QUEUE_CAPACITY 8192

time_wheel_t *time_wheel = NULL;

ring_buffer_t *syn_queue = NULL;
ring_buffer_t *retrans_queue = NULL;
ring_buffer_t *close_queue = NULL;

// 初始化
int task_queue_init();

// 启动时间轮
void launch();

// 第一种任务：重发建立连接过程中的第二次握手
typedef struct
{
    // 全体client的状态都放在一个大表里面，index是这个表里面的index
    int client_index;
    // 表明这是第几次重新发送第二次握手
    int retry_times;
} syn_task_t;

int add_syn_task(syn_task_t syn_task);

// 第二种任务：TCP重发
typedef struct
{
    // 重发不是以client进行的，而是以packet进行的
    // 也是需要一个表，这个表里面存放着全体packet
    int packet_index;

    int retry_times;
} retrans_task_t;

int add_retrans_task(retrans_task_t);

// 第三种任务：重发断开连接过程中的第二次挥手
int add_fin_task();

#endif