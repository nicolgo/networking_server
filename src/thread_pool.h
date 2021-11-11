#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "lib_gutil.h"

typedef struct thread_pool_struc{
    event_loop_struc *event_loop;
    int thread_number;
    int started;
    
}thread_pool_struc;

#endif