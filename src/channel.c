#include "lib_gutil.h"

channel_struc* channel_init(int fd, int events, event_read_callback_func event_read_callback,
    event_write_callback_func event_write_callback, void* data)
{
    channel_struc* channel = malloc(sizeof(channel_struc));
    channel->fd = fd;
    channel->events = events;
    channel->event_read_callback = event_read_callback;
    channel->event_write_callback = event_write_callback;
    channel->data = data;

    return channel;
}

int channel_write_event_is_enabled(channel_struc* channel)
{
    return channel->events & EVENT_WRITE;
}
int channel_write_event_enable(channel_struc* channel)
{
    event_loop_struc* event_loop = (event_loop_struc*)channel->data;
    channel->events = channel->events | EVENT_WRITE;
    event_loop_update_channel_event(event_loop, channel->fd, channel);

    return 0;
}
int channel_write_event_disable(channel_struc* channel)
{
    event_loop_struc* event_loop = (event_loop_struc*)channel->data;
    channel->events = channel->events | (~EVENT_WRITE);
    event_loop_update_channel_event(event_loop, channel->fd, channel);

    return 0;
}


/********************  channel map ***********************************/
int map_expand_space(channel_map_struc* map, int socket_fd, int msize)
{
    if (map->n_channel <= socket_fd) {
        int n_entries = map->n_channel ? map->n_channel : 32;
        void** tmp_channels;
        while (n_entries <= socket_fd) {
            n_entries <<= 1;
        }
        tmp_channels = (void**)realloc(map->channels, n_entries * msize);
        if (tmp_channels == NULL) {
            return -1;
        }
        memset(&tmp_channels[map->n_channel], 0, (n_entries - map->n_channel) * msize);
        map->n_channel = n_entries;
        map->channels = tmp_channels;
    }
    return 0;
}

void map_init(channel_map_struc* map)
{
    map->channels = NULL;
    map->n_channel = 0;
}

void map_free(channel_map_struc* map)
{
    int i;
    if (map->channels != NULL) {
        for (i = 0;i < map->n_channel;i++) {
            if (map->channels[i] != NULL) {
                free(map->channels[i]);
            }
        }
        free(map->channels);
        map->channels = NULL;
    }
    map->channels = 0;
}