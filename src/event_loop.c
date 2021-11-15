#include <pthread.h>
#include "lib_gutil.h"

static void event_loop_channel_buffer_noblock(event_loop_struc* event_loop,
    int fd, channel_struc* channel, int type);
static int event_loop_process_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel, int type);
static int event_loop_handle_pending_channel(event_loop_struc* event_loop);
static int handle_wakeup(void* data);

event_loop_struc* event_loop_init(char* thread_name) {
    channel_struc* channel;
    event_loop_struc* event_loop = malloc(sizeof(event_loop_struc));
    pthread_mutex_init(&event_loop->mutex, NULL);
    pthread_cond_init(&event_loop->cond, NULL);
    // set thread name.
    event_loop->owner_thread_id = pthread_self();
    if (thread_name != NULL) {
        event_loop->thread_name = thread_name;
    }
    else {
        event_loop->thread_name = "main thread";
    }

    event_loop->quit = 0;
    net_msgx("set epoll as dispatcher, %s", event_loop->thread_name);
    event_loop->event_dispatcher = &epoll_dispatcher;
    event_loop->event_dispatcher_data =
        event_loop->event_dispatcher->init(event_loop);

    event_loop->channel_map = malloc(sizeof(channel_map_struc));
    map_init(event_loop->channel_map);

    event_loop->is_handle_pending = 0;
    event_loop->pending_head = NULL;
    event_loop->pending_tail = NULL;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, event_loop->socker_pair) < 0) {
        perror("socketpair set failed");
    }

    channel = channel_init(event_loop->socker_pair[1], EVENT_READ,
        handle_wakeup, NULL, event_loop);
    event_loop_add_channel_event(event_loop, event_loop->socker_pair[1], channel);

    return event_loop;
}

int event_loop_run(event_loop_struc* event_loop)
{
    assert(event_loop != NULL);
    event_dispatcher_struc* dispatcher = event_loop->event_dispatcher;
    if (event_loop->owner_thread_id != pthread_self()) {
        perror("event loop thread is wrong");
        exit(EXIT_FAILURE);
    }
    net_msgx("event loop thread is %s", event_loop->thread_name);

    struct timeval time_val;
    time_val.tv_sec = 1;

    while (!event_loop->quit) {
        // block here to wait I/O event, and get active channel.
        dispatcher->dispatch(event_loop, &time_val);
        // handle the pending channel.
        event_loop_handle_pending_channel(event_loop);
    }

    net_msgx("event loop thread %s end.", event_loop->thread_name);

    return 0;
}

int channel_event_activate(event_loop_struc* event_loop, int fd, int events)
{
    channel_map_struc* map = event_loop->channel_map;
    net_msgx("activate channel fd==%d, events=%d, thread=%s",
        fd, events, event_loop->thread_name);

    if (fd < 0) {
        return 0;
    }
    if (fd >= map->n_channel) {
        return -1;
    }

    channel_struc* channel = map->channels[fd];
    assert(fd == channel->fd);

    if (events & (EVENT_READ)) {
        if (channel->event_read_callback) {
            channel->event_read_callback(channel->data);
        }
    }

    if (events & (EVENT_WRITE)) {
        if (channel->event_write_callback) {
            channel->event_write_callback(channel->data);
        }
    }

    return 0;
}

void event_loop_wakeup(event_loop_struc* event_loop)
{
    char one = 'a';
    ssize_t n = write(event_loop->socker_pair[0], &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("wakeup event loop thread failed");
    }
}
static int handle_wakeup(void* data)
{
    event_loop_struc* event_loop = (event_loop_struc*)data;
    char one;
    ssize_t n = read(event_loop->socker_pair[1], &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("handle wakeup failed");
    }
    net_msgx("wakeup, %s", event_loop->thread_name);
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

/****************** add/delete/update channel event ***********************/
int event_loop_process_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel, int type)
{
    pthread_mutex_lock(&event_loop->mutex);
    assert(event_loop->is_handle_pending == 0);
    event_loop_channel_buffer_noblock(event_loop, fd, channel, type);
    pthread_mutex_unlock(&event_loop->mutex);
    if (event_loop->owner_thread_id != pthread_self()) {
        event_loop_wakeup(event_loop);
    }
    else {
        event_loop_handle_pending_channel(event_loop);
    }
    return 0;
}
int event_loop_add_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    return event_loop_process_channel_event(event_loop, fd, channel, 1);
}
int event_loop_remove_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    return event_loop_process_channel_event(event_loop, fd, channel, 2);
}
int event_loop_update_channel_event(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    return event_loop_process_channel_event(event_loop, fd, channel, 3);
}

/****************** implemetation of channel ***********************/
static int event_loop_handle_pending_channel(event_loop_struc* event_loop)
{
    channel_elem_struc* channel_elem;
    pthread_mutex_lock(&event_loop->mutex);
    event_loop->is_handle_pending = 1;

    channel_elem = event_loop->pending_head;
    while (channel_elem != NULL) {
        channel_struc* channel = channel_elem->channel;
        int fd = channel->fd;
        switch (channel_elem->type)
        {
        case 1:
            event_loop_handle_pending_add(event_loop, fd, channel);
            break;
        case 2:
            event_loop_handle_pending_remove(event_loop, fd, channel);
            break;
        case 3:
            event_loop_handle_pending_update(event_loop, fd, channel);
            break;
        default:
            break;
        }
        channel_elem = channel_elem->next;
    }

    event_loop->pending_head = event_loop->pending_tail = NULL;
    event_loop->is_handle_pending = 0;
    pthread_mutex_unlock(&event_loop->mutex);
    return 0;
}

int event_loop_handle_pending_add(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    net_msgx("add channel fd == %d, %s", fd, event_loop->thread_name);
    channel_map_struc* channel_map = event_loop->channel_map;
    if (fd < 0) {
        return 0;
    }
    if (fd >= channel_map->n_channel) {
        if (map_expand_space(channel_map, fd, sizeof(channel_struc*)) == -1) {
            perror("failed to expand the map table");
            return -1;
        }
    }
    // add a key-channel pair.
    if (channel_map->channels[fd] == NULL) {
        event_dispatcher_struc* event_dispatcher = event_loop->event_dispatcher;
        channel_map->channels[fd] = channel;
        event_dispatcher->add(event_loop, channel);
        return 1;
    }

    return 0;
}
int event_loop_handle_pending_remove(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    int return_value = 0;
    channel_map_struc* channel_map = event_loop->channel_map;
    channel_struc* channel_tmp;
    if (fd < 0) {
        return 0;
    }
    if (fd >= channel_map->n_channel) {
        return -1;
    }

    channel_tmp = channel_map->channels[fd];

    if (event_loop->event_dispatcher->delete(event_loop, channel_tmp) == -1) {
        return_value = -1;
    }
    else {
        return_value = 1;
    }

    channel_map->channels[fd] = NULL;
    return return_value;
}
int event_loop_handle_pending_update(event_loop_struc* event_loop,
    int fd, channel_struc* channel)
{
    net_msgx("update channel fd == %d, %s", fd, event_loop->thread_name);
    channel_map_struc* channel_map = event_loop->channel_map;
    if (fd < 0) {
        return 0;
    }
    if (channel_map->channels[fd] == NULL) {
        return -1;
    }
    event_loop->event_dispatcher->update(event_loop, channel);
    return 0;
}


/******************** event loop thread **********************************/
int event_loop_thread_init(event_loop_thread_struc* event_loop_thread, int i)
{
    char buf[16] = { 0 };
    pthread_mutex_init(&event_loop_thread->mutex, NULL);
    pthread_cond_init(&event_loop_thread->cond, NULL);

    sprintf(buf, "Thread %d\0", i + 1);
    event_loop_thread->thread_name = buf;
    event_loop_thread->event_loop = NULL;
    event_loop_thread->thread_count = 0;
    event_loop_thread->thread_id = 0;

    return 0;
}

void* event_loop_thread_run(void* arg) {
    event_loop_thread_struc* event_loop_thread = (event_loop_thread_struc*)arg;

    pthread_mutex_lock(&event_loop_thread->mutex);

    event_loop_thread->event_loop = event_loop_init(event_loop_thread->thread_name);
    net_msgx("event loop thread init and signal, %s", event_loop_thread->thread_name);
    pthread_cond_signal(&event_loop_thread->cond);
    pthread_mutex_unlock(&event_loop_thread->mutex);

    event_loop_run(event_loop_thread->event_loop);

    return;
}

event_loop_struc* event_loop_thread_start(event_loop_thread_struc* event_loop_thread)
{
    pthread_create(&event_loop_thread->thread_id, NULL,
        &event_loop_thread_run, event_loop_thread);
    assert(pthread_mutex_lock(&event_loop_thread->mutex) == 0);
    while (event_loop_thread->event_loop == NULL) {
        assert(pthread_cond_wait(&event_loop_thread->cond, &event_loop_thread->mutex) == 0);
    }
    assert(pthread_mutex_unlock(&event_loop_thread->mutex) == 0);
    net_msgx("event loop thread started, %s", event_loop_thread->thread_name);
    return event_loop_thread->event_loop;
}