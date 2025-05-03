#pragma once
#include "rte_ethdev.h"
#include <rte_mbuf.h>
#include <stdint.h>

#define SERVER_PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define SEND_INTERVAL 1
#define HEARTBEAT_MSG "Server heartbeat"
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32
#define SRC_IP RTE_IPV4(192, 168, 1, 152)
#define PORT_ID (uint8_t)0

extern volatile int force_quit;
extern uint16_t port_id;
extern struct rte_mempool *mbuf_pool;

void global_init(int argc, char *argv[]);

