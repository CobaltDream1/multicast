#include <pthread.h>
#include "listener.h"
#include "sender.h"
#include "global.h"

int main(int argc, char *argv[])
{
    global_init(argc, argv);

    // 启动监听线程
    pthread_t listener_thread;
    pthread_create(&listener_thread, NULL, listen_client, NULL);

    // printf("listening started...\n");
    // printf("sending started...\n");
    // // 主循环：处理命令 发送消息
    // packet_sender();
    while(1) {

    }

    return 0;
}
