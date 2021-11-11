#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer_struc{
    char *data;
    int read_index;
    int write_index;
    int total_size;
}buffer_struc;

#endif