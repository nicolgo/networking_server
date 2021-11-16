#include "lib_gutil.h"
#include "event_dispatcher.h"

static void* epoll_init(event_loop_struc* event_loop);
static int epoll_add(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_delete(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_update(event_loop_struc* event_loop, channel_struc* channel);
static int epoll_dispatch(event_loop_struc* event_loop, struct timeval* time_val);
static void epoll_clear(event_loop_struc* event_loop);

static void* poll_init(event_loop_struc* event_loop);
static int poll_add(event_loop_struc* event_loop, channel_struc* channel);
static int poll_delete(event_loop_struc* event_loop, channel_struc* channel);
static int poll_update(event_loop_struc* event_loop, channel_struc* channel);
static int poll_dispatch(event_loop_struc* event_loop, struct timeval* time_val);
static void poll_clear(event_loop_struc* event_loop);

struct event_dispatcher_struc epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_delete,
    epoll_update,
    epoll_dispatch,
    epoll_clear,
};

struct event_dispatcher_struc poll_dispatcher = {
    "poll",
    poll_init,
    poll_add,
    poll_delete,
    poll_update,
    poll_dispatch,
    poll_clear,
};


/****************** epoll dispatcher ************************************/
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
            channel_event_activate(event_loop, epoll_dispatcher_data->events[i].data.fd,
                EVENT_READ);
        }

        if (epoll_dispatcher_data->events[i].events & EPOLLOUT) {
            net_msgx("get message channel fd=%d for write,%s",
                epoll_dispatcher_data->events[i].data.fd, event_loop->thread_name);
            channel_event_activate(event_loop, epoll_dispatcher_data->events[i].data.fd,
                EVENT_WRITE);
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

/************************** poll dispatcher ****************************/

static void* poll_init(event_loop_struc* event_loop)
{
    poll_dispatcher_data_struc* poll_dispatcher_data =
        malloc(sizeof(poll_dispatcher_data_struc));
    poll_dispatcher_data->event_set = malloc(sizeof(struct pollfd) * POLL_MAX_SIZE);
    for (int i = 0; i < POLL_MAX_SIZE;i++) {
        poll_dispatcher_data->event_set[i].fd = -1;
    }
    poll_dispatcher_data->event_count = 0;
    poll_dispatcher_data->nfds = 0;
    poll_dispatcher_data->realloc_copy = 0;

    return poll_dispatcher_data;
}
static int poll_add(event_loop_struc* event_loop, channel_struc* channel)
{
    int fd = channel->fd;
    int events = 0;
    poll_dispatcher_data_struc* poll_dispatcher_data =
        (poll_dispatcher_data_struc*)event_loop->event_dispatcher_data;
    if (channel->events & EVENT_READ) {
        events = events | POLLRDNORM;
    }

    if (channel->events & EVENT_WRITE) {
        events = events | POLLWRNORM;
    }

    int i = 0;
    for (i = 0; i < POLL_MAX_SIZE; i++) {
        if (poll_dispatcher_data->event_set[i].fd < 0) {
            poll_dispatcher_data->event_set[i].fd = fd;
            poll_dispatcher_data->event_set[i].events = events;
            break;
        }
    }

    net_msgx("poll added channel fd==%d, %s", fd, event_loop->thread_name);
    if (i >= POLL_MAX_SIZE) {
        LOG_ERROR("too many clients, just abort it");
    }

    return 0;
}
static int poll_delete(event_loop_struc* event_loop, channel_struc* channel)
{
    poll_dispatcher_data_struc* poll_dispatcher_data =
        (poll_dispatcher_data_struc*)event_loop->event_dispatcher_data;
    int fd = channel->fd;

    int i = 0;
    for (i = 0; i < POLL_MAX_SIZE; i++) {
        if (poll_dispatcher_data->event_set[i].fd == fd) {
            poll_dispatcher_data->event_set[i].fd = -1;
            break;
        }
    }

    net_msgx("poll delete channel fd==%d, %s", fd, event_loop->thread_name);
    if (i >= POLL_MAX_SIZE) {
        LOG_ERROR("can not find fd, poll delete error");
    }

    return 0;
}
static int poll_update(event_loop_struc* event_loop, channel_struc* channel)
{
    poll_dispatcher_data_struc* poll_dispatcher_data =
        (poll_dispatcher_data_struc*)event_loop->event_dispatcher_data;
    int fd = channel->fd;

    int events = 0;
    if (channel->events & EVENT_READ) {
        events = events | POLLRDNORM;
    }

    if (channel->events & EVENT_WRITE) {
        events = events | POLLWRNORM;
    }

    //找到fd对应的记录
    int i = 0;
    for (i = 0; i < POLL_MAX_SIZE; i++) {
        if (poll_dispatcher_data->event_set[i].fd == fd) {
            poll_dispatcher_data->event_set[i].events = events;
            break;
        }
    }

    net_msgx("poll updated channel fd==%d, %s", fd, event_loop->thread_name);
    if (i >= poll_dispatcher_data) {
        LOG_ERROR("can not find fd, poll updated error");
    }

    return 0;
}
static int poll_dispatch(event_loop_struc* event_loop, struct timeval* time_val)
{
    poll_dispatcher_data_struc* poll_dispatcher_data =
        (poll_dispatcher_data_struc*)event_loop->event_dispatcher_data;
    int ready_number = 0;
    int timewait = time_val->tv_sec * 1000;
    if ((ready_number = poll(poll_dispatcher_data->event_set, POLL_MAX_SIZE, timewait)) < 0) {
        perror("poll failed!");
        exit(EXIT_FAILURE);
    }

    if (ready_number <= 0) {
        return 0;
    }

    int i;
    for (i = 0; i < POLL_MAX_SIZE; i++) {
        int socket_fd;
        struct pollfd poll_fd = poll_dispatcher_data->event_set[i];
        if ((socket_fd = poll_fd.fd) < 0)
            continue;

        if (poll_fd.revents > 0) {
            net_msgx("get message channel i==%d, fd==%d, %s", i, socket_fd, event_loop->thread_name);

            if (poll_fd.revents & POLLRDNORM) {
                channel_event_activate(event_loop, socket_fd, EVENT_READ);
            }

            if (poll_fd.revents & POLLWRNORM) {
                channel_event_activate(event_loop, socket_fd, EVENT_WRITE);
            }

            if (--ready_number <= 0)
                break;
        }

    }

    return 0;
}
static void poll_clear(event_loop_struc* event_loop)
{
    poll_dispatcher_data_struc* poll_dispatcher_data =
        (poll_dispatcher_data_struc*)event_loop->event_dispatcher_data;

    if (poll_dispatcher_data != NULL) {
        if (poll_dispatcher_data->event_set != NULL) {
            free(poll_dispatcher_data->event_set);
            poll_dispatcher_data->event_set = NULL;
        }
        free(poll_dispatcher_data);
        event_loop->event_dispatcher_data = NULL;
    }
}