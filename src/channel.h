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

/* The index of channel map is socket descriptor.*/
typedef struct channel_map_struc{
    void **channels;//the address of channel object
    int n_channel;
}channel_map_struc;

int map_make_space(channel_map_struc *map,int socket_fd,int msize);
void map_init(channel_map_struc *map);
void map_free(channel_map_struc *map);

#endif
