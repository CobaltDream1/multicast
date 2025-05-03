#ifndef HEADER_BUFFER_H
#define HEADER_BUFFER_H

#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_ether.h>

typedef struct
{
    struct rte_ether_hdr ethernet_header;
    struct rte_ipv4_hdr ip_header;
    struct rte_tcp_hdr udp_header;
} header_t;

#define HEADER_SIZE sizeof(header_t)

typedef struct
{
    header_t *buffer;
    size_t size;
    size_t max;
} header_buffer_t;

int header_buffer_init(header_buffer_t *header_buf, size_t max);

int find_header_index(header_buffer_t *header_buf, uint32_t ip, uint16_t port);

int append_header(header_buffer_t *header_buf, uint32_t ip, uint16_t port, uint8_t mac[6]);

int modify_header(header_buffer_t *header_buf, int index, uint32_t ip, uint16_t port, uint8_t mac[6]);

int del_header(header_buffer_t *header_buf, int index);

#endif // HEADER_BUFFER_H