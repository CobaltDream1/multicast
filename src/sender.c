#include <stdio.h>
#include <unistd.h>
#include <rte_malloc.h>
#include "listener.h"
#include "global.h"
#include "sender.h"
#include "cmd_buffer.h"

static header_buffer_t header_buffer;

int sender_init(size_t max, uint32_t src_ip, uint32_t src_port)
{
    // 首先初始化header_buffer
    int result = header_buffer_init(&header_buffer, max);
    if (result != 1)
    {
        printf("failed to init header_buffer %d\n", result);
        return result;
    }

    // 然后给header_buffer填写初值，主要是src_ip, src_port这些
    for (int i = 0; i < max; i++)
    {
        header_t *cur_header = &header_buffer.buffer[i];
        struct rte_udp_hdr *udp = &cur_header->udp_header;
        struct rte_ipv4_hdr *ip = &cur_header->ip_header;
        struct rte_ether_hdr *eth = &cur_header->ethernet_header;

        // 填写UDP头部
        udp->src_port = rte_cpu_to_be_16(src_port);
        // udp->dgram_len = 0;
        udp->dgram_cksum = 0;

        // 填写IP头部
        ip->version_ihl = RTE_IPV4_VHL_DEF;
        ip->type_of_service = 0;
        // ip->total_length = 0;
        ip->packet_id = rte_cpu_to_be_16(0);
        ip->fragment_offset = 0;
        ip->time_to_live = 64;
        ip->next_proto_id = IPPROTO_UDP;
        ip->src_addr = rte_cpu_to_be_32(SRC_IP);
        // ip->dst_addr = rte_cpu_to_be_32(dst_ip);
        ip->hdr_checksum = 0;
        // 填写以太网头部
        struct rte_ether_addr src_mac;
        rte_eth_macaddr_get(PORT_ID, &src_mac);
        eth->src_addr = src_mac;
        eth->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
    }

    return 1;
}

// 非scatter-gather的实现
// void send_udp_packet(char *payload, size_t payload_size)
// {
//     size_t total_packets = header_buffer.size;
//     uint8_t *headers_base = (uint8_t *)header_buffer.buffer;

//     struct rte_mbuf **tx_mbufs;
//     tx_mbufs = malloc(sizeof(struct rte_mbuf *) * total_packets);
//     if (tx_mbufs == NULL)
//         rte_exit(EXIT_FAILURE, "Cannot allocate tx_mbuf array\n");

//     // 先分配所有mbuf
//     for (uint32_t i = 0; i < total_packets; i++) {
//         struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
//         if (mbuf == NULL)
//             rte_exit(EXIT_FAILURE, "Failed to alloc mbuf\n");

//         tx_mbufs[i] = mbuf;
//     }

//     // 第一次循环：拷贝header
//     for (uint32_t i = 0; i < total_packets; i++) {
//         struct rte_mbuf *mbuf = tx_mbufs[i];
//         uint8_t *dst_ptr = rte_pktmbuf_mtod(mbuf, uint8_t *);
//         uint8_t *header_src = headers_base + i * HEADER_SIZE;

//         rte_memcpy(dst_ptr, header_src, HEADER_SIZE);
//         mbuf->data_len = HEADER_SIZE;
//         mbuf->pkt_len  = HEADER_SIZE;
//         mbuf->l2_len = sizeof(struct rte_ether_hdr); // L2层长度 (Ethernet头)
//         mbuf->l3_len = sizeof(struct rte_ipv4_hdr);  // L3层长度 (IPv4头)

//         mbuf->ol_flags |= RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_UDP_CKSUM;

//         // 修改header里面的length信息
//         struct rte_ether_hdr *eth_hdr = (struct rte_ether_hdr *)dst_ptr;
//         struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
//         struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
//         ip_hdr->total_length = rte_cpu_to_be_16(sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + payload_size);
//         udp_hdr->dgram_len = rte_cpu_to_be_16(sizeof(struct rte_udp_hdr) + payload_size);
//         // 把checksum计算卸载到网卡上
//         ip_hdr->hdr_checksum = 0;
//         udp_hdr->dgram_cksum = 0;
//     }

//     // 第二次循环：追加payload
//     for (uint32_t i = 0; i < total_packets; i++) {
//         struct rte_mbuf *mbuf = tx_mbufs[i];
//         uint8_t *dst_ptr = rte_pktmbuf_mtod(mbuf, uint8_t *);

//         rte_memcpy(dst_ptr + HEADER_SIZE, payload, payload_size);
//         mbuf->data_len += payload_size;
//     }
//     uint32_t sent = 0;

//     while (sent < total_packets) {
//         uint16_t burst = RTE_MIN(BURST_SIZE, total_packets - sent);

//         uint16_t nb_tx = rte_eth_tx_burst(port_id, 0,
//                                           &tx_mbufs[sent], burst);
//         sent += nb_tx;
//     }
// }

void send_udp_packet(char *payload, size_t payload_size)
{
    size_t total_packets = header_buffer.size;
    uint8_t *headers_base = (uint8_t *)header_buffer.buffer;

    struct rte_mbuf **tx_mbufs = malloc(sizeof(struct rte_mbuf *) * total_packets);
    if (tx_mbufs == NULL)
        rte_exit(EXIT_FAILURE, "Cannot allocate tx_mbuf array\n");

    // 准备 payload mbuf（所有包共用一个）
    struct rte_mbuf *payload_mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if (payload_mbuf == NULL)
        rte_exit(EXIT_FAILURE, "Failed to alloc payload mbuf\n");
    char *payload_mbuf_pointer = rte_pktmbuf_mtod(payload_mbuf, char *);
    rte_memcpy(payload_mbuf_pointer, payload, payload_size);

    payload_mbuf->data_len = payload_size;
    payload_mbuf->pkt_len = payload_size;
    payload_mbuf->nb_segs = 1;
    payload_mbuf->next = NULL;
    payload_mbuf->ol_flags = 0;

    // 手动增加引用计数（因为多个包共用）
    rte_mbuf_refcnt_update(payload_mbuf, total_packets);

    // 分配所有 header mbuf
    for (uint32_t i = 0; i < total_packets; i++)
    {
        struct rte_mbuf *hdr_mbuf = rte_pktmbuf_alloc(mbuf_pool);
        if (hdr_mbuf == NULL)
            rte_exit(EXIT_FAILURE, "Failed to alloc header mbuf\n");

        // 把header挂成外部缓冲区
        uint8_t *header_ptr = headers_base + i * HEADER_SIZE;
        rte_iova_t iova = rte_malloc_virt2iova(headers_base) + i * HEADER_SIZE;
        rte_pktmbuf_attach_extbuf(hdr_mbuf, header_ptr, iova,
                                  HEADER_SIZE, NULL);

        hdr_mbuf->data_len = HEADER_SIZE;
        hdr_mbuf->pkt_len = HEADER_SIZE;
        hdr_mbuf->nb_segs = 1;
        hdr_mbuf->next = NULL;

        hdr_mbuf->l2_len = sizeof(struct rte_ether_hdr);
        hdr_mbuf->l3_len = sizeof(struct rte_ipv4_hdr);

        hdr_mbuf->ol_flags = RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_UDP_CKSUM;

        // 修改header里面的length字段
        struct rte_ether_hdr *eth_hdr = (struct rte_ether_hdr *)header_ptr;
        struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
        struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
        ip_hdr->total_length = rte_cpu_to_be_16(sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + payload_size);
        udp_hdr->dgram_len = rte_cpu_to_be_16(sizeof(struct rte_udp_hdr) + payload_size);
        ip_hdr->hdr_checksum = 0;
        udp_hdr->dgram_cksum = 0;

        // 链接 header + payload
        hdr_mbuf->next = payload_mbuf;
        hdr_mbuf->pkt_len += payload_size;
        hdr_mbuf->nb_segs = 2;

        tx_mbufs[i] = hdr_mbuf;
    }
    uint32_t sent = 0;
    while (sent < total_packets)
    {
        uint16_t burst = RTE_MIN(BURST_SIZE, total_packets - sent);
        uint16_t nb_tx = rte_eth_tx_burst(port_id, 0, &tx_mbufs[sent], burst);
        sent += nb_tx;
    }

    free(tx_mbufs);

    // 最后把payload mbuf减掉refcnt
    rte_pktmbuf_free(payload_mbuf);
}

// 向所有客户端发送心跳包
void packet_sender()
{
    char *payload = "Hello, multicast!!!";
    size_t payload_size = strlen(payload);
    while (1)
    {
        sleep(SEND_INTERVAL);
        handle_cmd(&header_buffer);
        send_udp_packet(payload, payload_size);
    }
}