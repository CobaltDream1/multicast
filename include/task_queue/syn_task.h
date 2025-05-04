#include "task_queue.h"
#include "buffer_pool.h"
class client_info_t;

// 第一种任务：重发建立连接过程中的第二次握手
struct syn_task_t : public task_t
{
    // 全体client的状态都放在一个map里面，用ip和port作为key
    uint32_t ip;
    uint16_t port;

    void try_syn_retrans(client_info_t *client_info);
    void confirm_connect(client_info_t *client_info);

public:
    void handle() override;
};

using syn_task_buffer_t = buffer_pool_t<syn_task_t>;