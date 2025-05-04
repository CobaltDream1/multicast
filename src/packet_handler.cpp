#include "packet_handler.h"
#include <rte_ip.h>
#include <rte_ether.h>
#include <rte_tcp.h>
#include <rte_arp.h>
#include "global.h"
#include "sender.h"

int packet_handler_t::get_pkt_type(struct rte_mbuf *mbuf)
{
    printf("received a new pkt.\n");
    struct rte_ether_hdr *eth_hdr;
    struct rte_ipv4_hdr *ip_hdr;
    struct rte_tcp_hdr *tcp_hdr;
    uint8_t *pkt_data;
    pkt_data = rte_pktmbuf_mtod(mbuf, uint8_t *);
    eth_hdr = (struct rte_ether_hdr *)pkt_data;

    if (RTE_BE16(eth_hdr->ether_type) == RTE_ETHER_TYPE_ARP)
    {
        return pkt_type_enum::ARP_REQ;
    }

    ip_hdr = (struct rte_ipv4_hdr *)(pkt_data + sizeof(struct rte_ether_hdr));

    if (ip_hdr->next_proto_id != IPPROTO_IP)
    {
        printf("Not a TCP packet: %u\n", ip_hdr->next_proto_id);
        return pkt_type_enum::NO_SUPPORT;
    }

    tcp_hdr = (struct rte_tcp_hdr *)((uint8_t *)ip_hdr + sizeof(struct rte_ipv4_hdr));

    uint32_t ip = ip_hdr->src_addr;
    uint16_t port = tcp_hdr->src_port;
    uint8_t mac[6];
    memcpy(mac, eth_hdr->src_addr.addr_bytes, 6);
    printf("TCP packet: %u.%u.%u.%u:%u",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF, ip & 0xFF, port);
    if (ip_hdr->dst_addr != SERVER_IP || tcp_hdr->dst_port != SERVER_PORT)
    {
        printf("target is not correct %u %u\n", ip_hdr->dst_addr, tcp_hdr->dst_port);
        return NO_SUPPORT;
    }

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
    return NO_SUPPORT;
}

int packet_handler_t::handle_arp_req(struct rte_mbuf *mbuf)
{
    int result = sender_t::reply_arp_req(mbuf);
    printf("handle arp req.%d\n", result);

    return result;
}

int packet_handler_t::handle_tcp_syn(struct rte_mbuf *mbuf)
{
    return 0;
}

int packet_handler_t::handle_tcp_syn_confirm(struct rte_mbuf *mbuf)
{
    return 0;
}

int packet_handler_t::handle_tcp_ack(struct rte_mbuf *mbuf)
{
    return 0;
}

int packet_handler_t::handle_packet(struct rte_mbuf *mbuf)
{
    int type = get_pkt_type(mbuf);
    if (type == -1)
    {
        printf("invalid type\n");
        return -1;
    }

    int result = 0;
    switch (type)
    {
    case ARP_REQ:
        result = handle_arp_req(mbuf);
        break;
    case TCP_SYN:
        result = handle_tcp_syn(mbuf);
        break;
    case TCP_SYN_CONFIRM:
        result = handle_tcp_syn_confirm(mbuf);
        break;
    default:
        result = -1;
    }

    return result;
}