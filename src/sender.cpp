#include <stdio.h>
#include <unistd.h>
#include <rte_malloc.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_ether.h>
#include "listener.h"
#include "global.h"
#include "sender.h"
#include "cmd_buffer.h"

// static header_buffer_t header_buffer;

int sender_init()
{
    return 1;
}
#define ARP_HDRLEN 28
#define ETHER_TYPE_ARP 0x0806
#define ARP_REQUEST 1
#define ARP_REPLY 2

int sender_t::reply_arp_req(struct rte_mbuf *pkt)
{
    struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    struct rte_arp_hdr *arp = (struct rte_arp_hdr *)(eth_hdr + 1);

    if (arp->arp_opcode != rte_cpu_to_be_16(RTE_ARP_OP_REQUEST) ||
        arp->arp_data.arp_tip != rte_cpu_to_be_32(SERVER_IP)) {
    printf("arp: %u %u %u %u\n", arp->arp_opcode, arp->arp_data.arp_tip, rte_cpu_to_be_16(RTE_ARP_OP_REQUEST), rte_cpu_to_be_32(SERVER_IP));
        return -1;
    }

    struct rte_mbuf *reply = rte_pktmbuf_alloc(pkt->pool);
    if (!reply)
    {
        printf("failed to allocate mbuf for arp reply.\n");
        return -1;
    }

    struct rte_ether_hdr *eth_reply = rte_pktmbuf_mtod(reply, struct rte_ether_hdr *);
    struct rte_arp_hdr *arp_reply = (struct rte_arp_hdr *)(eth_reply + 1);

    // Ethernet header
    eth_reply->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_ARP);
    rte_ether_addr_copy(&arp->arp_data.arp_sha, &eth_reply->dst_addr);
    memcpy(&eth_reply->src_addr, SERVER_MAC, 6);

    // ARP reply
    arp_reply->arp_hardware = rte_cpu_to_be_16(RTE_ARP_HRD_ETHER);
    arp_reply->arp_protocol = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
    arp_reply->arp_hlen = RTE_ETHER_ADDR_LEN;
    arp_reply->arp_plen = sizeof(uint32_t);
    arp_reply->arp_opcode = rte_cpu_to_be_16(RTE_ARP_OP_REPLY);
    memcpy(&arp_reply->arp_data.arp_sha, SERVER_MAC, 6);
    arp_reply->arp_data.arp_sip = rte_cpu_to_be_32(SERVER_IP);
    rte_ether_addr_copy(&arp->arp_data.arp_sha, &arp_reply->arp_data.arp_tha);
    arp_reply->arp_data.arp_tip = arp->arp_data.arp_sip;

    reply->data_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_arp_hdr);
    reply->pkt_len = reply->data_len;

    uint16_t result = rte_eth_tx_burst(port_id, 0, &reply, 1);
    return (int)result;
}

// // 非scatter-gather的实现
// // void send_udp_packet(char *payload, size_t payload_size)
// // {
// //     size_t total_packets = header_buffer.size;
// //     uint8_t *headers_base = (uint8_t *)header_buffer.buffer;

// //     struct rte_mbuf **tx_mbufs;
// //     tx_mbufs = malloc(sizeof(struct rte_mbuf *) * total_packets);
// //     if (tx_mbufs == nullptr)
// //         rte_exit(EXIT_FAILURE, "Cannot allocate tx_mbuf array\n");

// //     // 先分配所有mbuf
// //     for (uint32_t i = 0; i < total_packets; i++) {
// //         struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
// //         if (mbuf == nullptr)
// //             rte_exit(EXIT_FAILURE, "Failed to alloc mbuf\n");

// //         tx_mbufs[i] = mbuf;
// //     }

// //     // 第一次循环：拷贝header
// //     for (uint32_t i = 0; i < total_packets; i++) {
// //         struct rte_mbuf *mbuf = tx_mbufs[i];
// //         uint8_t *dst_ptr = rte_pktmbuf_mtod(mbuf, uint8_t *);
// //         uint8_t *header_src = headers_base + i * HEADER_SIZE;

// //         rte_memcpy(dst_ptr, header_src, HEADER_SIZE);
// //         mbuf->data_len = HEADER_SIZE;
// //         mbuf->pkt_len  = HEADER_SIZE;
// //         mbuf->l2_len = sizeof(struct rte_ether_hdr); // L2层长度 (Ethernet头)
// //         mbuf->l3_len = sizeof(struct rte_ipv4_hdr);  // L3层长度 (IPv4头)

// //         mbuf->ol_flags |= RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_UDP_CKSUM;

// //         // 修改header里面的length信息
// //         struct rte_ether_hdr *eth_hdr = (struct rte_ether_hdr *)dst_ptr;
// //         struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
// //         struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
// //         ip_hdr->total_length = rte_cpu_to_be_16(sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + payload_size);
// //         udp_hdr->dgram_len = rte_cpu_to_be_16(sizeof(struct rte_udp_hdr) + payload_size);
// //         // 把checksum计算卸载到网卡上
// //         ip_hdr->hdr_checksum = 0;
// //         udp_hdr->dgram_cksum = 0;
// //     }

// //     // 第二次循环：追加payload
// //     for (uint32_t i = 0; i < total_packets; i++) {
// //         struct rte_mbuf *mbuf = tx_mbufs[i];
// //         uint8_t *dst_ptr = rte_pktmbuf_mtod(mbuf, uint8_t *);

// //         rte_memcpy(dst_ptr + HEADER_SIZE, payload, payload_size);
// //         mbuf->data_len += payload_size;
// //     }
// //     uint32_t sent = 0;

// //     while (sent < total_packets) {
// //         uint16_t burst = RTE_MIN(BURST_SIZE, total_packets - sent);

// //         uint16_t nb_tx = rte_eth_tx_burst(port_id, 0,
// //                                           &tx_mbufs[sent], burst);
// //         sent += nb_tx;
// //     }
// // }

// void send_udp_packet(const char *payload, size_t payload_size)
// {
//     size_t total_packets = header_buffer.size;
//     uint8_t *headers_base = (uint8_t *)header_buffer.buffer;

//     struct rte_mbuf **tx_mbufs = (struct rte_mbuf **)malloc(sizeof(struct rte_mbuf *) * total_packets);
//     if (tx_mbufs == nullptr)
//         rte_exit(EXIT_FAILURE, "Cannot allocate tx_mbuf array\n");

//     // 准备 payload mbuf（所有包共用一个）
//     struct rte_mbuf *payload_mbuf = rte_pktmbuf_alloc(mbuf_pool);
//     if (payload_mbuf == nullptr)
//         rte_exit(EXIT_FAILURE, "Failed to alloc payload mbuf\n");
//     char *payload_mbuf_pointer = rte_pktmbuf_mtod(payload_mbuf, char *);
//     rte_memcpy(payload_mbuf_pointer, payload, payload_size);

//     payload_mbuf->data_len = payload_size;
//     payload_mbuf->pkt_len = payload_size;
//     payload_mbuf->nb_segs = 1;
//     payload_mbuf->next = nullptr;
//     payload_mbuf->ol_flags = 0;

//     // 手动增加引用计数（因为多个包共用）
//     rte_mbuf_refcnt_update(payload_mbuf, total_packets);

//     // 分配所有 header mbuf
//     for (uint32_t i = 0; i < total_packets; i++)
//     {
//         struct rte_mbuf *hdr_mbuf = rte_pktmbuf_alloc(mbuf_pool);
//         if (hdr_mbuf == nullptr)
//             rte_exit(EXIT_FAILURE, "Failed to alloc header mbuf\n");

//         // 把header挂成外部缓冲区
//         uint8_t *header_ptr = headers_base + i * HEADER_SIZE;
//         rte_iova_t iova = rte_malloc_virt2iova(headers_base) + i * HEADER_SIZE;
//         rte_pktmbuf_attach_extbuf(hdr_mbuf, header_ptr, iova,
//                                   HEADER_SIZE, nullptr);

//         hdr_mbuf->data_len = HEADER_SIZE;
//         hdr_mbuf->pkt_len = HEADER_SIZE;
//         hdr_mbuf->nb_segs = 1;
//         hdr_mbuf->next = nullptr;

//         hdr_mbuf->l2_len = sizeof(struct rte_ether_hdr);
//         hdr_mbuf->l3_len = sizeof(struct rte_ipv4_hdr);

//         hdr_mbuf->ol_flags = RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_UDP_CKSUM;

//         // 修改header里面的length字段
//         struct rte_ether_hdr *eth_hdr = (struct rte_ether_hdr *)header_ptr;
//         struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
//         struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
//         ip_hdr->total_length = rte_cpu_to_be_16(sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + payload_size);
//         udp_hdr->dgram_len = rte_cpu_to_be_16(sizeof(struct rte_udp_hdr) + payload_size);
//         ip_hdr->hdr_checksum = 0;
//         udp_hdr->dgram_cksum = 0;

//         // 链接 header + payload
//         hdr_mbuf->next = payload_mbuf;
//         hdr_mbuf->pkt_len += payload_size;
//         hdr_mbuf->nb_segs = 2;

//         tx_mbufs[i] = hdr_mbuf;
//     }
//     uint32_t sent = 0;
//     while (sent < total_packets)
//     {
//         uint16_t burst = RTE_MIN(BURST_SIZE, total_packets - sent);
//         uint16_t nb_tx = rte_eth_tx_burst(port_id, 0, &tx_mbufs[sent], burst);
//         sent += nb_tx;
//     }

//     free(tx_mbufs);

//     // 最后把payload mbuf减掉refcnt
//     rte_pktmbuf_free(payload_mbuf);
// }

// // 向所有客户端发送心跳包
// // void packet_sender()
// // {
// //     const char *payload = "Hello, multicast!!!";
// //     size_t payload_size = strlen(payload);
// //     while (1)
// //     {
// //         sleep(SEND_INTERVAL);
// //         handle_cmd(&header_buffer);
// //         send_udp_packet(payload, payload_size);
// //     }
// // }