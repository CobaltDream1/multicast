#include "syn_task.h"
#include "task_queue.h"
#include "client_buffer.h"

#define SYN_RETRY_LIMIT 6

void syn_task_t::try_syn_retrans(client_info_t *client_info)
{
    // 查看重发次数是否已经达到上限，
    if (client_info->syn_retry_times >= SYN_RETRY_LIMIT)
    {
        printf("failed to connect with client: %u %u\n", ip, port);

        // 从client_buffer中回收
        client_buffer_t::get_instance().remove(ip, port);
    }
    else
    {
        // 重发，放置任务
        syn_task_t *task = new syn_task_t;
        task->time = 1 << client_info->syn_retry_times;
        task->ip = ip;
        task->port = port;

        int result = task_queue_t::get_instance().add_task(task);
        if (result != 1)
        {
            printf("falied to add task: %d\n", result);
        }
        client_info->syn_retry_times++;
    }
}

void syn_task_t::confirm_connect(client_info_t *client_info)
{
    client_info->state = CONNECTED;
}

void syn_task_t::handle()
{
    // 到时间了，检测我们的client的第三次握手到底有没有传过来
    client_buffer_t CB = client_buffer_t::get_instance();

    client_info_t *client_info = CB.find(ip, port);
    if (client_info == nullptr)
    {
        printf("warning: client not found in buffer: %u %u", ip, port);
        return;
    }

    // 还处于等待建立连接的ACK的状态中，
    if (client_info->state == SYN)
    {
        try_syn_retrans(client_info);
    }
    // 已经收到了第三次握手
    else
    {

    }
}