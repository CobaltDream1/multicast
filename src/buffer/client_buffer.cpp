#include "client_buffer.h"

const size_t ack_ring_buffer_size = 8192;

void ack_state_t::init()
{
    if (this->rb == nullptr)
    {
        this->rb = new ring_buffer_t;
        this->rb->init(ack_ring_buffer_size, sizeof(ack_state_t));
    }
    else
    {
        // 上一个client留下来的你就直接用吧，ack_rb的生命周期是和位置绑定的，不是和client绑定的
        this->rb->reset();
    }
}

void ack_state_t::append(uint32_t length)
{
}

void ack_state_t::ack(uint32_t ack_num)
{
}
inline client_key_t client_buffer_make_key(uint32_t ip, uint16_t port)
{
    return (static_cast<uint64_t>(ip) << 16) | port;
}

client_buffer_t &client_buffer_t::get_instance()
{
    static client_buffer_t client_buffer;

    return client_buffer;
}

int client_buffer_t::insert(uint32_t ip, uint16_t port)
{
    if (this->available.empty())
    {
        printf("no available place for new client in client buffer.\n");
        return -1;
    }

    client_key_t key = client_buffer_make_key(ip, port);
    size_t pos = this->available.top();
    this->available.pop();

    this->buffer[pos].reset(ip, port);
    this->index.insert(std::pair<client_key_t, size_t>(key, pos));

    return pos;
}

client_info_t *client_buffer_t::find(uint32_t ip, uint16_t port)
{
    client_key_t key = client_buffer_make_key(key, port);
    auto it = this->index.find(key);
    if (it == this->index.end())
    {
        return nullptr;
    }
    return &buffer[it->second];
}

void client_buffer_t::remove(uint32_t ip, uint16_t port)
{
    client_key_t key = client_buffer_make_key(key, port);
    auto it = this->index.find(key);
    if (it == this->index.end())
    {
        printf("not found in client_buffer: %u %u\n", ip, port);
        return;
    }

    this->index.erase(key);
    this->available.push(it->second);
}