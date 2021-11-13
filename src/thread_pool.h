#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "lib_gutil.h"

typedef struct thread_pool_struc{
    event_loop_struc *main_loop;
    int thread_number;
    int started;
    event_loop_thread_struc *event_loop_threads;
    int position;
}thread_pool_struc;

thread_pool_struc* thread_pool_init(event_loop_struc *main_loop, int thread_number);

void thread_pool_start(thread_pool_struc* thread_pool);

event_loop_struc* thread_pool_get_loop(thread_pool_struc* thread_pool);

#endif