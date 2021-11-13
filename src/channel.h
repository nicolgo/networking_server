#ifndef CHANNEL_H
#define CHANNEL_H

#include "lib_gutil.h"

#define EVENT_TIMEOUT    0x01
/** Wait for a socket or FD to become readable */
#define EVENT_READ        0x02
/** Wait for a socket or FD to become writeable */
#define EVENT_WRITE    0x04
/** Wait for a POSIX signal to be raised*/
#define EVENT_SIGNAL    0x08


// write and read event callback
typedef int (*event_write_callback_func)(void* data);
typedef int (*event_read_callback_func)(void* data);

typedef struct channel_struc {
    int fd;
    int events;//the event type

    event_write_callback_func event_write_callback;
    event_read_callback_func event_read_callback;
    void* data;//the data for callback
} channel_struc;

channel_struc* channel_new(int fd, int events, event_read_callback_func event_read_callback,
    event_write_callback_func event_write_callback, void* data);

int channel_write_event_is_enabled(channel_struc* channel);
int channel_write_event_enable(channel_struc* channel);
int channel_write_event_disable(channel_struc* channel);

/* The index of channel map is socket descriptor.*/
typedef struct channel_map_struc {
    void** channels;//the address of channel object
    int n_channel;
}channel_map_struc;

int map_make_space(channel_map_struc* map, int socket_fd, int msize);
void map_init(channel_map_struc* map);
void map_free(channel_map_struc* map);

#endif
