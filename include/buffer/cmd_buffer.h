#pragma once
#include "header_buffer.h"

struct cmd_add_t
{
    uint32_t ip;
    uint16_t port;
    uint8_t mac[6];
};

struct cmd_del_t
{
    uint32_t ip;
    uint16_t port;
};

struct ring_param_t
{
    size_t write_index;
    size_t read_index;
    size_t max;
};

struct cmd_add_buffer_t
{
    cmd_add_t *buffer;
    ring_param_t ring_param;
};

 struct cmd_del_buffer_t
{
    cmd_del_t *buffer;
    ring_param_t ring_param;
};

// 初始化command ring buffer
int init_cmd_buffer(size_t size);

// 添加一条add命令，给listener调用
int add_client(uint32_t ip, uint16_t port, uint8_t mac[6]);

// 添加一条del命令，给listener调用
int del_client(uint32_t ip, uint16_t port);

// 处理ringbuffer中存在的命令，给sender调用
int handle_cmd(header_buffer_t *header_buffer);
