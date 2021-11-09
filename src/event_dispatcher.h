#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include "lib_gutil.h"

typedef struct event_dispatcher_struc {
    const char* name;
    /*init function*/
    void* (*init)(struct event_loop* event_loop, struct channel* channel);

    int (*add)(struct event_loop* event_loop, struct channel* channel)
} event_dispatcher_struc;

#endif