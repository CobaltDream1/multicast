#include "cmd_buffer.h"
#include "header_buffer.h"

int buffer_is_full(ring_param_t *ring_param)
{
    return (ring_param->write_index + 1) % ring_param->max == ring_param->read_index;
}

int buffer_is_empty(ring_param_t *ring_param)
{
    return ring_param->read_index == ring_param->write_index;
}

cmd_add_buffer_t *init_cmd_add_buffer(size_t size)
{
    cmd_add_buffer_t *rb = malloc(sizeof(cmd_add_buffer_t));
    if (!rb)
        return NULL;

    rb->buffer = malloc(sizeof(cmd_add_t) * size);
    if (!rb->buffer)
    {
        free(rb);
        return NULL;
    }

    rb->ring_param.max = size;
    rb->ring_param.read_index = 0;
    rb->ring_param.write_index = 0;

    return rb;
}

void free_cmd_add_buffer(cmd_add_buffer_t *b)
{
    if (b != NULL)
    {
        if (b->buffer != NULL)
        {
            free(b->buffer);
        }
        free(b);
    }
}

void free_cmd_del_buffer(cmd_del_buffer_t *b)
{
    if (b != NULL)
    {
        if (b->buffer != NULL)
        {
            free(b->buffer);
        }
        free(b);
    }
}

cmd_del_buffer_t *init_cmd_del_buffer(size_t size)
{
    cmd_del_buffer_t *rb = malloc(sizeof(cmd_del_buffer_t));
    if (!rb)
        return NULL;

    rb->buffer = malloc(sizeof(cmd_del_t) * size);
    if (!rb->buffer)
    {
        free(rb);
        return NULL;
    }

    rb->ring_param.max = size;
    rb->ring_param.read_index = 0;
    rb->ring_param.write_index = 0;

    return rb;
}

// 把一条新的add命令加入到ring_buffer，成功时返回1
int enqueue_cmd_add(cmd_add_buffer_t *cmd_add_buffer, uint32_t ip, uint16_t port, uint8_t mac[6])
{
    if (buffer_is_full(&cmd_add_buffer->ring_param))
    {
        return 0;
    }

    ring_param_t *ring_param = &cmd_add_buffer->ring_param;
    cmd_add_buffer->buffer[ring_param->write_index].ip = ip;
    cmd_add_buffer->buffer[ring_param->write_index].port = port;
    memcpy(cmd_add_buffer->buffer[ring_param->write_index].mac, mac, 6);

    ring_param->write_index = (ring_param->write_index + 1) % ring_param->max;
    return 1;
}

// 从ring_buffer里面取出一条add命令，成功时返回1
int dequeue_cmd_add(cmd_add_buffer_t *cmd_add_buffer, cmd_add_t *cmd_add)
{
    if (buffer_is_empty(&cmd_add_buffer->ring_param))
    {
        return 0;
    }

    ring_param_t *ring_param = &cmd_add_buffer->ring_param;
    cmd_add->ip = cmd_add_buffer->buffer[ring_param->write_index].ip;
    cmd_add->port = cmd_add_buffer->buffer[ring_param->write_index].port;
    memcpy(cmd_add->mac, cmd_add_buffer->buffer[ring_param->write_index].mac, 6);

    ring_param->read_index = (ring_param->read_index + 1) % ring_param->max;
    return 1;
}

// 把一条新的del命令加入到ring_buffer，成功时返回1
int enqueue_cmd_del(cmd_del_buffer_t *cmd_del_buffer, uint32_t ip, uint16_t port)
{
    if (buffer_is_full(&cmd_del_buffer->ring_param))
    {
        return 0;
    }

    ring_param_t *ring_param = &cmd_del_buffer->ring_param;
    cmd_del_buffer->buffer[ring_param->write_index].ip = ip;
    cmd_del_buffer->buffer[ring_param->write_index].port = port;

    ring_param->write_index = (ring_param->write_index + 1) % ring_param->max;
    return 1;
}

// 从ring_buffer里面取出一条del命令，成功时返回1
int dequeue_cmd_del(cmd_del_buffer_t *cmd_del_buffer, cmd_del_t *cmd_del)
{
    if (buffer_is_empty(&cmd_del_buffer->ring_param))
    {
        return 0;
    }

    ring_param_t *ring_param = &cmd_del_buffer->ring_param;
    cmd_del->ip = cmd_del_buffer->buffer[ring_param->write_index].ip;
    cmd_del->port = cmd_del_buffer->buffer[ring_param->write_index].port;

    ring_param->read_index = (ring_param->read_index + 1) % ring_param->max;
    return 1;
}

//------------------------------------------------------------------------

static cmd_add_buffer_t *cmd_add_rb;
static cmd_del_buffer_t *cmd_del_rb;

int init_cmd_buffer(size_t size)
{
    if (cmd_add_rb != NULL || cmd_del_rb != NULL)
    {
        printf("cmd_add_rb or cmd_del_rb already allocated\n");
        return 0;
    }

    cmd_add_rb = init_cmd_add_buffer(size);
    if (cmd_add_rb == NULL)
    {
        printf("failed to allocate cmd_add_rb\n");
        return 0;
    }
    cmd_del_rb = init_cmd_del_buffer(size);
    if (cmd_del_rb == NULL)
    {
        printf("failed to allocate cmd_del_rb\n");
        return 0;
    }
}

int add_client(uint32_t ip, uint16_t port, uint8_t mac[6])
{
    if (cmd_add_rb == NULL)
    {
        printf("cmd_add_rb has not allocated\n");
        return 0;
    }

    int result = enqueue_cmd_add(cmd_add_rb, ip, port, mac);
    if (result != 1)
    {
        printf("failed to enqueue cmd add: %d\n", result);
        return result;
    }

    return 1;
}

int del_client(uint32_t ip, uint16_t port)
{
    if (cmd_del_rb == NULL)
    {
        printf("cmd_del_rb has not allocated\n");
        return 0;
    }

    int result = enqueue_cmd_del(cmd_del_rb, ip, port);
    if (result != 1)
    {
        printf("failed to enqueue cmd del: %d\n", result);
        return result;
    }

    return 1;
}

int handle_cmd(header_buffer_t *header_buffer)
{
    if (cmd_add_rb == NULL || cmd_del_rb == NULL)
    {
        printf("rb not allocated");
        return -1;
    }

    if (buffer_is_empty(&cmd_add_rb->ring_param) && buffer_is_empty(&cmd_del_rb->ring_param))
    {
        return 0;
    }

    // 首先确定本次要apply哪些命令
    size_t add_buffer_write = cmd_add_rb->ring_param.write_index;
    size_t add_buffer_read = cmd_add_rb->ring_param.read_index;
    size_t del_buffer_write = cmd_del_rb->ring_param.write_index;
    size_t del_buffer_read = cmd_del_rb->ring_param.read_index;

    // 然后确定这些命令的数量
    size_t add_buffer_max = cmd_add_rb->ring_param.max;
    size_t add_num = (add_buffer_write + add_buffer_max - cmd_add_rb->ring_param.read_index) % add_buffer_max;
    size_t del_buffer_max = cmd_del_rb->ring_param.max;
    size_t del_num = (del_buffer_write + del_buffer_max - cmd_del_rb->ring_param.read_index) % del_buffer_max;

    // 对于del命令需要找到对应的index
    int *del_index_list = malloc(del_num * sizeof(int));
    for (size_t i = 0; i < del_num; i++)
    {
        cmd_del_t *cmd_del = &cmd_del_rb->buffer[(del_buffer_read + i) % del_buffer_max];
        int pos = find_header_index(header_buffer, cmd_del->ip, cmd_del->port);
        if (pos == -1)
        {
            printf("del target not found\n");
        }
        del_index_list[i] = pos;
    }

    // 双指针 成对处理add命令和del命令
    size_t i = 0;
    size_t j = 0;
    for (; i < add_num && j < del_num; i++, j++)
    {
        // 直接覆盖
        int target = del_index_list[j];
        cmd_add_t *cmd_add = &cmd_add_rb->buffer[(add_buffer_read + i) % add_buffer_max];
        if (target != -1)
        {
            int result = modify_header(header_buffer, target, cmd_add->ip, cmd_add->port, cmd_add->mac);
            if (result != 1)
            {
                printf("failed to modify, %d\n", target);
            }
        }
        else
        {
            int result = append_header(header_buffer, cmd_add->ip, cmd_add->port, cmd_add->mac);
            if (result != 1)
            {
                printf("failed to append\n");
            }
        }
    }

    // 处理剩余的add命令
    for (; i < add_num; i++)
    {
        cmd_add_t *cmd_add = &cmd_add_rb->buffer[(add_buffer_read + i) % add_buffer_max];
        int result = append_header(header_buffer, cmd_add->ip, cmd_add->port, cmd_add->mac);
        if (result != 1)
        {
            printf("failed to append\n");
        }
    }

    // 处理剩余的del命令
    for (; j < del_num; j++)
    {
        int target = del_index_list[j];
        if (target != -1)
        {
            int result = del_header(header_buffer, target);
            if (result != 1)
            {
                printf("failed to del, %d\n", target);
            }
        }
    }

    // 最后从ring_buffer移除掉这些已经被处理的命令
    cmd_add_rb->ring_param.read_index = add_buffer_write;
    cmd_del_rb->ring_param.read_index = del_buffer_write;


    return 1;
}
