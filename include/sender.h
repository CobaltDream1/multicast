#pragma once
#include "header_buffer.h"

class sender_t
{
public:
    // 这个是即时发送
    static int send_one(struct rte_mbuf *mbuf);

    // 响应arp请求
    static int reply_arp_req(struct rte_mbuf *mbuf);

    // 发送第二次握手，初次发和重发都用这个
    static int send_syn_ack();

    // 这个是多播发送，是第一次发送不是重传
    static int send_multicast(char *payload, size_t payload_size);

    // 这个是多播重传，index是payload_buffer里面的编号
    static int retrans_multicast(int index);
};
