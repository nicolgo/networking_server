#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include "lib_gutil.h"
#include <pthread.h>

/* a linked list of channel*/
typedef struct channel_elem_struc{
    int type;// 1 add; 2 delete
    channel_struc *channel;
    channel_elem_struc *next;
}channel_elem_struc;

typedef struct event_loop_struc {
    int quit;
    event_dispatcher_struc *event_dispatcher;
    void *event_dispatcher_data;// void* type can be transfered easily.
    channel_map_struc *channel_map;

    int is_handle_pending;
    channel_elem_struc *pending_head;
    channel_elem_struc *pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int sockerPair[2];
    char *thread_name;

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
int event_loop_run(event_loop_struc *event_loop);

int event_loop_thread_init(event_loop_thread_struc* event_loop_thread, int i);

event_loop_struc* event_loop_thread_start(event_loop_thread_struc* event_loop_thread);

#endif