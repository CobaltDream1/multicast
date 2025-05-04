#include "cmd_buffer.h"
#include "global.h"
#include "client_buffer.h"
#include "task_queue.h"
#include "syn_task.h"
#include "packet_handler.h"


inline uint32_t get_ip(struct rte_mbuf *mbuf)
{
    uint8_t *pkt_data = rte_pktmbuf_mtod(mbuf, uint8_t *);
    struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(pkt_data + sizeof(struct rte_ether_hdr));
    return ip_hdr->src_addr;
}

inline int16_t get_port(struct rte_mbuf *mbuf)
{
    uint8_t *pkt_data = rte_pktmbuf_mtod(mbuf, uint8_t *);
    struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *)(pkt_data + sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
    uint16_t port = tcp_hdr->src_port;

    return port;
}

// 放置发送第二次握手的任务
static int put_syn_ack_task(uint32_t ip, uint16_t port)
{
    int index = syn_task_buffer_t::get_instance().allocate();
    if (index == -1)
    {
        printf("failed to allocate syn task from buffer.\n");
        return -1;
    }

    syn_task_t *task = syn_task_buffer_t::get_instance().find(index);
    task->time = 0;
    task->ip = ip;
    task->port = port;

    task_queue_t::get_instance().add_task(task);

}

static int handle_tcp_syn(struct rte_mbuf *mbuf)
{
    // 加入到client_buffer，发包，放置重传检测任务
    client_buffer_t &CB = client_buffer_t::get_instance();
    // 先看看有没有
    uint32_t ip = get_ip(mbuf);
    uint32_t port = get_port(mbuf);
    client_info_t *client_info = CB.find(ip, port);

    if(client_info != nullptr) {
        return -1;
    }

    CB.insert(ip, port);

    return 0;
}

// 监听新用户的加入，将对应的命令写入ring_buffer
void *listen_client(void *p)
{
    struct rte_mbuf *bufs[BURST_SIZE];
    uint16_t nb_rx;

    // uint8_t mac[6] = {0xb4, 0x96, 0x91, 0xb2, 0xac, 0x51};

    while (!force_quit)
    {
        nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
        if (nb_rx > 0)
        {
            for (int i = 0; i < nb_rx; i++)
            {
                int result = packet_handler_t::handle_packet(bufs[i]);
            }
        }
    }

    return nullptr;
}