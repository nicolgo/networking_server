#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include "lib_gutil.h"
#include <pthread.h>

typedef struct event_loop_struc {
    int quit;
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