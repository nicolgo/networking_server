#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H
#include "event_loop.h"
#include "lib_gutil.h"
#include <poll.h>

#define MAXEVENTS 128
#define POLL_MAX_SIZE 1024

extern struct event_dispatcher_struc epoll_dispatcher;
extern struct event_dispatcher_struc poll_dispatcher;

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

typedef struct poll_dispatcher_data_struc{
    int event_count;
    int nfds;
    int realloc_copy;
    struct pollfd* event_set;
    struct pollfd* event_set_copy;
}poll_dispatcher_data_struc;


#endif