#pragma once
#include <rte_mbuf.h>

enum pkt_type_enum
{
    ARP_REQ,
    TCP_SYN,
    TCP_SYN_CONFIRM,
    TCP_ACK,
    TCP_CLOSE,
    TCP_CLOSE_CONFIRM,
    NO_SUPPORT,
};

class packet_handler_t
{
private:
    static int get_pkt_type(struct rte_mbuf *mbuf);

    static int handle_arp_req(struct rte_mbuf *mbuf);

    static int handle_tcp_syn(struct rte_mbuf *mbuf);
    static int handle_tcp_syn_confirm(struct rte_mbuf *mbuf);
    static int handle_tcp_ack(struct rte_mbuf *mbuf);

public:
    static int handle_packet(struct rte_mbuf *mbuf);
};
