#include "lib_gutil.h"

event_loop_struc* event_loop_init(char* thread_name) {
    event_loop_struc* event_loop = malloc(sizeof(event_loop_struc));
    pthread_mutex_init(&event_loop->mutex,NULL);
    pthread_cond_init(&event_loop->cond,NULL);

    if(thread_name != NULL){
        event_loop->thread_name = thread_name;
    }else{
        event_loop->thread_name = "main thread";
    }

    
    return event_loop;
}

int event_loop_thread_init(event_loop_thread_struc* event_loop_thread, int i)
{
    char buf[16] = { 0 };
    pthread_mutex_init(&event_loop_thread->mutex, NULL);
    pthread_cont_init(&event_loop_thread->cond, NULL);

    sprintf(buf, "Thread %d\0", i + 1);
    event_loop_thread->thread_name = buf;
    event_loop_thread->event_loop = NULL;
    event_loop_thread->thread_count = 0;
    event_loop_thread->thread_id = 0;

    return 0;
}