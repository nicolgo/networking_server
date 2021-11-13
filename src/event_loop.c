#include "lib_gutil.h"

static void event_loop_channel_buffer_noblock(event_loop_struc* event_loop,
    int fd, channel_struc* channel, int type);

event_loop_struc* event_loop_init(char* thread_name) {
    event_loop_struc* event_loop = malloc(sizeof(event_loop_struc));
    pthread_mutex_init(&event_loop->mutex, NULL);
    pthread_cond_init(&event_loop->cond, NULL);

    event_loop->quit = 0;
    event_loop->event_dispatcher = &epoll_dispatcher;
    event_loop->event_dispatcher_data =
        event_loop->event_dispatcher->init(event_loop);
    event_loop->channel_map = mallco(sizeof(channel_map_struc));
    map_init(event_loop->channel_map);

    event_loop->is_handle_pending = 0;
    event_loop->pending_head = NULL;
    event_loop->pending_tail = NULL;

    event_loop->owner_thread_id = pthread_self();
    if (thread_name != NULL) {
        event_loop->thread_name = thread_name;
    }
    else {
        event_loop->thread_name = "main thread";
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, event_loop->socker_pair) < 0) {
        perror("socketpair set failed");
    }

    return event_loop;
}

static void event_loop_channel_buffer_noblock(event_loop_struc* event_loop,
    int fd, channel_struc* channel, int type)
{
    channel_elem_struc* channel_elem = malloc(sizeof(channel_elem));
    channel_elem->channel = channel;
    channel_elem->type = type;
    channel_elem->next = NULL;
    if (event_loop->pending_head == NULL) {
        event_loop->pending_head = event_loop->pending_tail = channel_elem;
    }
    else {
        event_loop->pending_tail->next = channel_elem;
        event_loop->pending_tail = channel_elem;
    }
}

int event_loop_process_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel,int type)
{
    pthread_mutex_lock(&event_loop->mutex);
    assert(event_loop->is_handle_pending == 0);
    event_loop_channel_buffer_noblock(event_loop,fd,channel,type);
    pthread_mutex_unlock(&event_loop->mutex);
    if(event_loop->owner_thread_id != pthread_self()){

    }else{

    }
    return 0;
}
int event_loop_add_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_remove_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);
int event_loop_update_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel);

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