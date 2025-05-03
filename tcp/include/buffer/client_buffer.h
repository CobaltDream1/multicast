#include <stdint.h>

// 收到了第一次握手，但是还没有收到第三次握手
#define SYN 0
// 三次握手已经结束，正式建立连接
#define CONNECTED 1
// 收到了第一次挥手，准备断开连接
#define FIN 2

typedef struct
{
    uint32_t ip;
    uint16_t port;

    int state;
    
    int last_packet_number;
    int expect_seq;
} client_info_t;
