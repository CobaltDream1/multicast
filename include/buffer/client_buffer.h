#pragma once
#include <stdint.h>
#include <unordered_map>
#include <stack>
#include "ring_buffer.h"

enum client_state
{
    // 收到了第一次握手，但是还没有收到第三次握手
    SYN,
    // 三次握手已经结束，正式建立连接
    CONNECTED,
    // 收到了第一次挥手，准备断开连接
    FIN
};

struct pending_ack_t
{
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;
    uint32_t timestamp = 0;
    uint32_t ack_count = 0;
};

class ack_state_t
{
    ring_buffer_t *rb = nullptr;

public:
    void init();
    void append(uint32_t length);
    void ack(uint32_t ack_num);
};

struct client_info_t
{
    uint32_t ip;
    uint16_t port;

    int state;

    uint32_t last_packet_number;
    uint32_t expect_seq;
    // 表明这是第几次重新发送第二次握手
    int syn_retry_times;

    ack_state_t ack_state;

    void reset(uint32_t ip, uint16_t port) {
        this->ip = ip;
        this->port = port;
        this->state = SYN;
        this->last_packet_number = 0;
        this->expect_seq = 0;

        ack_state.init();
    }
};

using client_key_t = uint64_t;

class client_buffer_t
{
    client_info_t *buffer;
    std::unordered_map<client_key_t, size_t> index;
    std::stack<size_t> available;

public:
    int insert(uint32_t ip, uint16_t port);
    client_info_t *find(uint32_t ip, uint16_t port);
    void remove(uint32_t ip, uint16_t port);

    static client_buffer_t &get_instance();
};
