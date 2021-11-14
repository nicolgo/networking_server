#include "lib_gutil.h"

static void* epoll_init(event_loop_struc* event_loop);
static int epoll_add(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_delete(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_update(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_dispatch(event_loop_struc* event_loop, struct timeval* time_val);
static void epoll_clear(event_loop_struc* event_loop);

const struct event_dispatcher_struc epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_delete,
    epoll_update,
    epoll_dispatch,
    epoll_clear,
};

static void* epoll_init(event_loop_struc* event_loop)
{
    epoll_dispatcher_data_struc* epoll_dispatcher_data = malloc(sizeof(epoll_dispatcher_data_struc));

    epoll_dispatcher_data->event_count = 0;
    epoll_dispatcher_data->nfds = 0;
    epoll_dispatcher_data->realloc_copy = 0;
    epoll_dispatcher_data->efd = 0;

    epoll_dispatcher_data->efd = epoll_create1(0);
    if (epoll_dispatcher_data->efd == -1) {
        perror("epoll create failed");
    }

    epoll_dispatcher_data->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    return epoll_dispatcher_data;
}

static int epoll_action(event_loop_struc* event_loop,
    channel_struc* channel, int action)
{
    int fd = channel->fd;
    int events = 0;
    struct epoll_event event;
    epoll_dispatcher_data_struc* epoll_dispatcher_data =
        (epoll_dispatcher_data_struc*)event_loop->event_dispatcher_data;

    event.data.fd = fd;
    if (channel->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (channel->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }
    event.events = events;
    if (epoll_ctl(epoll_dispatcher_data->efd, action, fd, &event) == -1) {
        perror("epoll_ctl operate fd failed");
    }

    return 0;
}

static int epoll_add(event_loop_struc* event_loop, channel_struc* channel)
{
    return epoll_action(event_loop, channel, EPOLL_CTL_ADD);
}
static int epoll_delete(event_loop_struc* event_loop, channel_struc* channel)
{
    return epoll_action(event_loop, channel, EPOLL_CTL_DEL);
}
static int epoll_update(event_loop_struc* event_loop, channel_struc* channel)
{
    return epoll_action(event_loop, channel, EPOLL_CTL_MOD);
}
static int epoll_dispatch(event_loop_struc* event_loop, struct timeval* time_val)
{
    int i, n;
    epoll_dispatcher_data_struc* epoll_dispatcher_data =
        (epoll_dispatcher_data_struc*)event_loop->event_dispatcher_data;

    n = epoll_wait(epoll_dispatcher_data->efd, epoll_dispatcher_data->events,
        MAXEVENTS, -1);
    net_msgx("epoll_wait wakeup, %s", event_loop->thread_name);

    for (i = 0;i < n;i++) {
        if ((epoll_dispatcher_data->events[i].events & EPOLLERR)
            || (epoll_dispatcher_data->events[i].events & EPOLLHUP)) {
            fprintf(stderr, "epoll error\n");
            close(epoll_dispatcher_data->events[i].data.fd);
            continue;
        }

        if (epoll_dispatcher_data->events[i].events & EPOLLIN) {
            net_msgx("get message channel fd=%d for read,%s",
                epoll_dispatcher_data->events[i].data.fd, event_loop->thread_name);

        }

        if (epoll_dispatcher_data->events[i].events & EPOLLIN) {
            net_msgx("get message channel fd=%d for write,%s",
                epoll_dispatcher_data->events[i].data.fd, event_loop->thread_name);

        }
    }
}
static void epoll_clear(event_loop_struc* event_loop)
{
    epoll_dispatcher_data_struc* epoll_dispatcher_data =
        (epoll_dispatcher_data_struc*)event_loop->event_dispatcher_data;
    if (epoll_dispatcher_data != NULL) {
        if (epoll_dispatcher_data->events != NULL) {
            free(epoll_dispatcher_data->events);
        }
        if (epoll_dispatcher_data->efd != 0) {
            close(epoll_dispatcher_data->efd);
        }
        free(epoll_dispatcher_data);
        event_loop->event_dispatcher_data = NULL;
    }

    return;
}