#include "header_buffer.h"
#include <rte_malloc.h>

int header_buffer_init(header_buffer_t *header_buf, size_t max)
{
    header_buf->buffer = (header_t *)rte_malloc(nullptr, max * sizeof(header_t), 0);
    if (header_buf->buffer == nullptr) {
        printf("failed to allocate memory for header_buffer\n");
        return -1;
    }

    header_buf->max = max;
    header_buf->size = 0;

    return 1;
}

int find_header_index(header_buffer_t *header_buf, uint32_t ip, uint16_t port)
{
    for (size_t i = 0; i < header_buf->size; i++)
    {
        struct rte_ipv4_hdr *ip_header = &header_buf->buffer[i].ip_header;
        struct rte_tcp_hdr *tcp_header = &header_buf->buffer[i].tcp_header;
        if (ip_header->dst_addr == ip && tcp_header->dst_port == port)
        {
            return i;
        }
    }

    return -1;
}

int append_header(header_buffer_t *header_buf, uint32_t ip, uint16_t port, uint8_t mac[6])
{
    if (header_buf->size == header_buf->max)
    {
        printf("no space for appending\n");
        return -1;
    }

    memcpy(header_buf->buffer[header_buf->size].ethernet_header.dst_addr.addr_bytes, mac, 6);
    header_buf->buffer[header_buf->size].ip_header.dst_addr = rte_cpu_to_be_32(ip);
    header_buf->buffer[header_buf->size].tcp_header.dst_port = rte_cpu_to_be_16(port);

    header_buf->size += 1;

    return 1;
}

int modify_header(header_buffer_t *header_buf, int index, uint32_t ip, uint16_t port, uint8_t mac[6])
{
    if (index >= header_buf->size || index < 0)
    {
        printf("modify index out of range");
        return -1;
    }

    memcpy(header_buf->buffer[index].ethernet_header.dst_addr.addr_bytes, mac, 6);
    header_buf->buffer[index].ip_header.dst_addr = rte_cpu_to_be_32(ip);
    header_buf->buffer[index].tcp_header.dst_port = rte_cpu_to_be_16(port);
}

int del_header(header_buffer_t *header_buf, int index)
{
    if (header_buf->size == 0)
    {
        printf("no header to delete\n");
        return -1;
    }
    if (index >= header_buf->size || index < 0)
    {
        printf("delete index out of range");
        return -1;
    }

    memcpy(header_buf->buffer[index].ethernet_header.dst_addr.addr_bytes, header_buf->buffer[header_buf->size - 1].ethernet_header.dst_addr.addr_bytes, 6);
    uint32_t ip = header_buf->buffer[header_buf->size - 1].ip_header.dst_addr;
    header_buf->buffer[header_buf->size].ip_header.dst_addr = ip;
    uint16_t port = header_buf->buffer[header_buf->size - 1].tcp_header.dst_port;
    header_buf->buffer[header_buf->size].tcp_header.dst_port = port;

    header_buf->size -= 1;

    return 1;
}
