#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "lib_gutil.h"

typedef struct event_loop_struc{
    int quit;
} event_loop_struc;

event_loop_struc *event_loop_init(char *thread_name);

#endif