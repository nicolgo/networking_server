#ifndef CHANNEL_H
#define CHANNEL_H

// write and read event callback
typedef int (*event_write_callback)(void* data);
typedef int (*event_read_callback)(void* data);

typedef struct channel_struc {
    int fd;
    int events;

    event_write_callback eventWriteCallback;
    event_read_callback eventReadCallback;
    void* data;//the data for callback
} channel_struc;


#endif
