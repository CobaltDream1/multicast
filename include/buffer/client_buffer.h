#pragma once
#include <stdint.h>

enum client_state
{
    // 收到了第一次握手，但是还没有收到第三次握手
    SYN,
    // 三次握手已经结束，正式建立连接
    CONNECTED,
    // 收到了第一次挥手，准备断开连接
    FIN
};

struct client_info_t
{
    uint32_t ip;
    uint16_t port;

    int state;

    int last_packet_number;
    int expect_seq;
};
