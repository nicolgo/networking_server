#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H
#include "lib_gutil.h"

#define MAXEVENTS 128

typedef struct event_dispatcher_struc {
    const char* name;

    /*init function*/
    void* (*init)(event_loop_struc* event_loop);

    int (*add)(event_loop_struc* event_loop, channel_struc* channel);

    int (*delete)(event_loop_struc* event_loop, channel_struc* channel);

    int (*update)(event_loop_struc* event_loop, channel_struc* channel);

    int (*dispatch)(event_loop_struc* event_loop, struct timeval* time_val);

    void (*clear)(event_loop_struc* event_loop);

} event_dispatcher_struc;

typedef struct epoll_dispatcher_data_struc {
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event* events;
}epoll_dispatcher_data_struc;



#endif