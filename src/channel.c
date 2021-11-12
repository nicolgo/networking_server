#include "lib_gutil.h"

int map_make_space(channel_map_struc* map, int socket_fd, int msize)
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