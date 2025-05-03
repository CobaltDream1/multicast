#include "cmd_buffer.h"
#include "global.h"

// 现在是根据内容去判断加入还是删除，后面可以改成根据timeout删除

enum tcp_pkt_type
{
    TCP_SYN,
    TCP_SYN_CONFIRM,
    TCP_ACK,
    TCP_CLOSE,
    TCP_CLOSE_CONFIRM,
    TCP_NO_SUPPORT,
};

static int
get_tcp_pkt_type(struct rte_mbuf *mbuf)
{
    struct rte_ether_hdr *eth_hdr;
    struct rte_ipv4_hdr *ip_hdr;
    struct rte_tcp_hdr *tcp_hdr;
    uint8_t *pkt_data;

    pkt_data = rte_pktmbuf_mtod(mbuf, uint8_t *);

    eth_hdr = (struct rte_ether_hdr *)pkt_data;
    // if (RTE_BE16(eth_hdr->ether_type) != RTE_ETHER_TYPE_IPV4)
    // {
    //     printf("Not an IPv4 packet\n");
    //     return -1;
    // }

    ip_hdr = (struct rte_ipv4_hdr *)(pkt_data + sizeof(struct rte_ether_hdr));

    // if (ip_hdr->next_proto_id != IPPROTO_UDP)
    // {
    //     printf("Not a UDP packet\n");
    //     return -1;
    // }

    tcp_hdr = (struct rte_udp_hdr *)((uint8_t *)ip_hdr + sizeof(struct rte_ipv4_hdr));

    if (ip_hdr->dst_addr != SRC_IP || tcp_hdr->dst_port != SERVER_PORT)
    {
        printf("target is not correct %u %u\n", ip_hdr->dst_addr, tcp_hdr->dst_port);
        return -1;
    }

    uint32_t ip = ip_hdr->src_addr;
    uint16_t port = tcp_hdr->src_port;
    uint8_t mac[6];
    memcpy(mac, eth_hdr->src_addr.addr_bytes, 6);
    printf("TCP packet: %u.%u.%u.%u:%u",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF, ip & 0xFF, port);
    uint32_t flag = tcp_hdr->tcp_flags;

    if (flag & RTE_TCP_SYN_FLAG)
    {
        return TCP_SYN;
    }
    if (flag & RTE_TCP_ACK_FLAG && tcp_hdr->recv_ack == 1)
    {
        return TCP_SYN_CONFIRM;
    }
    if (flag & RTE_TCP_ACK_FLAG && tcp_hdr->recv_ack != 1)
    {
        return TCP_ACK;
    }
    // TODO: close

    printf("received tcp pkt type no support\n");
    return TCP_NO_SUPPORT;
}

static int handle_tcp_syn(struct rte_mbuf *mbuf)
{
    return 0;
}

static int handle_tcp_syn_confirm(struct rte_mbuf *mbuf)
{
}

static int handle_recv_tcp_pkt(struct rte_mbuf *mbuf)
{
    int type = get_tcp_pkt_type(mbuf);
    if (type == -1)
    {
        printf("invalid type\n");
        return -1;
    }

    switch (type)
    {
    case TCP_SYN:
        break;
    case TCP_SYN_CONFIRM:
        break;
    }
}

// 监听新用户的加入，将对应的命令写入ring_buffer
void *listen_client(void *p)
{
    struct rte_mbuf *bufs[BURST_SIZE];
    uint16_t nb_rx;

    // uint8_t mac[6] = {0xb4, 0x96, 0x91, 0xb2, 0xac, 0x51};
    // add_client(RTE_IPV4(192, 168, 1, 151), 1111, mac);
    // add_client(RTE_IPV4(192, 168, 1, 151), 2222, mac);
    // add_client(RTE_IPV4(192, 168, 1, 151), 3333, mac);
    // add_client(RTE_IPV4(192, 168, 1, 151), 4444, mac);
    // add_client(RTE_IPV4(192, 168, 1, 151), 5555, mac);

    while (!force_quit)
    {
        nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
        if (nb_rx > 0)
        {
            for (int i = 0; i < nb_rx; i++)
            {
                int type = get_tcp_pkt_type(bufs[i]);
            }
        }
    }

    return NULL;
}