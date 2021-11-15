#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "event_dispatcher.h"
#include "lib_gutil.h"
#include <pthread.h>

extern const struct event_dispatcher_struc epoll_dispatcher;

/* a linked list of channel*/
typedef struct channel_elem_struc {
    int type;// 1 add; 2 delete
    channel_struc* channel;
    struct channel_elem_struc* next;
}channel_elem_struc;

typedef struct event_loop_struc {
    int quit;
    char* thread_name;
    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    struct event_dispatcher_struc* event_dispatcher;
    void* event_dispatcher_data;// epoll or poll.
    channel_map_struc* channel_map;

    int is_handle_pending;
    // the event of child thread from head to tail
    channel_elem_struc* pending_head;
    channel_elem_struc* pending_tail;

    int socker_pair[2];
} event_loop_struc;

typedef struct event_loop_thread_struct {
    char* thread_name;
    long thread_count;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    event_loop_struc* event_loop;
}event_loop_thread_struc;

event_loop_struc* event_loop_init(char* thread_name);
int event_loop_run(event_loop_struc* event_loop);
void event_loop_wakeup(event_loop_struc* event_loop);

int channel_event_activate(event_loop_struc* event_loop, int fd, int res);
int event_loop_add_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_remove_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_update_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);

int event_loop_handle_pending_add(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_handle_pending_remove(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_handle_pending_update(event_loop_struc* event_loop,
    int fd, channel_struc* channel);

int channel_event_activate(event_loop_struc* event_loop, int fd, int events);

int event_loop_thread_init(event_loop_thread_struc* event_loop_thread, int i);
event_loop_struc* event_loop_thread_start(event_loop_thread_struc* event_loop_thread);

#endif