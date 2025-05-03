#include "task_queue.h"
#include <stdio.h>

int task_queue_init()
{
    int result = 0;
    time_wheel = new time_wheel_t;

    result = time_wheel->time_wheel_init( SLOT_COUNT, TICK_INTERVAL);
    if (result != 1)
    {
        printf("failed to init time_wheel\n");
        return -1;
    }

    syn_queue = new ring_buffer_t;
    // result = ring_buffer_init(syn_queue, 8192, sizeof());
    return 1;
}

