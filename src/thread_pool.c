#include "thread_pool.h"
#include "lib_gutil.h"

thread_pool_struc* thread_pool_init(event_loop_struc *main_loop, int thread_number)
{
    thread_pool_struc* thread_pool = malloc(sizeof(thread_pool_struc));
    thread_pool->main_loop = main_loop;
    thread_pool->position = 0;
    thread_pool->thread_number = thread_number;
    thread_pool->started = 0;
    thread_pool->event_loop_threads = NULL;
    return thread_pool;
}

void thread_pool_start(thread_pool_struc* thread_pool)
{
    void *tmp;
    thread_pool->started = 1;

    if(thread_pool->thread_number <= 0){
        return 0;
    }

    thread_pool->event_loop_threads = malloc(thread_pool->thread_number*sizeof(event_loop_thread_struc));
    for(int i = 0;i<thread_pool->thread_number;i++){
        event_loop_thread_init(&thread_pool->event_loop_threads[i],i);
        event_loop_thread_start(&thread_pool->event_loop_threads[i]);
    }
}

event_loop_struc* thread_pool_get_loop(thread_pool_struc* thread_pool)
{
    event_loop_struc* selected = thread_pool->main_loop;

    if(thread_pool->thread_number > 0){
        selected = thread_pool->event_loop_threads[thread_pool->position].event_loop;
        if(++(thread_pool->position) >= thread_pool->thread_number){
            thread_pool->position = 0;
        }
    }

    return selected;
}