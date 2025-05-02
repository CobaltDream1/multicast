#include "global.h"
#include "sender.h"
#include "cmd_buffer.h"

volatile int force_quit = 0;
uint16_t port_id = 0;
struct rte_mempool *mbuf_pool = NULL;

static struct rte_eth_conf port_conf = {
    .txmode = {
        .offloads = DEV_TX_OFFLOAD_IPV4_CKSUM |
                    DEV_TX_OFFLOAD_UDP_CKSUM |
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

    rte_eth_rx_queue_setup(PORT_ID, 0, 128, rte_eth_dev_socket_id(PORT_ID), NULL, mbuf_pool);
    rte_eth_tx_queue_setup(PORT_ID, 0, 128, rte_eth_dev_socket_id(PORT_ID), NULL);
    rte_eth_dev_start(PORT_ID);
}

void global_init(int argc, char *argv[])
{
    dpdk_init(argc, argv);
    init_cmd_buffer(BUFFER_SIZE);
    sender_init(BUFFER_SIZE, SRC_IP, SERVER_PORT);
}