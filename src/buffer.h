#ifndef BUFFER_H
#define BUFFER_H

#define INIT_BUFFER_SIZE 65536

typedef struct buffer_struc{
    char *data;
    int read_index;
    int write_index;
    int total_size;
}buffer_struc;

buffer_struc *buffer_new();
void buffer_free(buffer_struc* buffer);

int buffer_writeable_size(buffer_struc *buffer);
int buffer_readable_size(buffer_struc *buffer);
int buffer_front_spare_size(buffer_struc *buffer);

int buffer_append(buffer_struc *buffer,void *data,int size);
int buffer_append_char(buffer_struc* buffer,char c);
int buffer_append_string(buffer_struc *buffer,char *data);
int buffer_socket_read(buffer_struc *buffer,int fd);
char buffer_read_char(buffer_struc *buffer);
char *buffer_find_CRLF(buffer_struc *buffer);

#endif