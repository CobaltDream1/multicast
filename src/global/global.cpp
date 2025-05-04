#include "global.h"
#include "sender.h"
#include "cmd_buffer.h"
#include "syn_task.h"
volatile int force_quit = 0;
uint16_t port_id = 0;
struct rte_mempool *mbuf_pool = nullptr;

static struct rte_eth_conf port_conf = {
    .txmode = {
        .offloads = DEV_TX_OFFLOAD_IPV4_CKSUM |
                    DEV_TX_OFFLOAD_TCP_CKSUM |
                    DEV_TX_OFFLOAD_MULTI_SEGS,
    },
};

static void dpdk_init(int argc, char *argv[])
{
    // 初始化EAL
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL init\n");

    // 创建mbuf池
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * 2,
                                        MBUF_CACHE_SIZE, 0,
                                        RTE_MBUF_DEFAULT_BUF_SIZE,
                                        rte_socket_id());
    if (!mbuf_pool)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    // 配置端口
    ret = rte_eth_dev_configure(PORT_ID, 1, 1, &port_conf);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d\n", ret);

    rte_eth_rx_queue_setup(PORT_ID, 0, 512, rte_eth_dev_socket_id(PORT_ID), nullptr, mbuf_pool);
    rte_eth_tx_queue_setup(PORT_ID, 0, 512, rte_eth_dev_socket_id(PORT_ID), nullptr);

    rte_eth_tx_queue_setup(PORT_ID, 1, 512, rte_eth_dev_socket_id(PORT_ID), nullptr);
    rte_eth_dev_start(PORT_ID);
}

void init_buffer_pool()
{
    syn_task_buffer_t::get_instance().init(SYN_TASK_BUFFER_CAPACITY);
}

void global_init(int argc, char *argv[])
{
    dpdk_init(argc, argv);
    init_cmd_buffer(BUFFER_SIZE);
    init_buffer_pool();
    // sender_init(BUFFER_SIZE, SERVER_IP, SERVER_PORT);
}