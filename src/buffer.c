#include "lib_gutil.h"
#include <sys/uio.h>

const char* CRLF = "\r\n";

buffer_struc* buffer_new()
{
    buffer_struc* buffer = malloc(sizeof(buffer_struc));
    if (buffer == NULL) {
        return NULL;
    }

    buffer->data = (char*)malloc(INIT_BUFFER_SIZE);
    buffer->total_size = INIT_BUFFER_SIZE;
    buffer->read_index = 0;
    buffer->write_index = 0;
    return buffer;
}

void buffer_free(buffer_struc* buffer)
{
    if (buffer != NULL) {
        if (buffer->data != NULL) {
            free(buffer->data);
        }
        free(buffer);
    }
}

int buffer_writeable_size(buffer_struc* buffer)
{
    return buffer->total_size - buffer->write_index;
}
int buffer_readable_size(buffer_struc* buffer)
{
    return buffer->write_index - buffer->read_index;
}
int buffer_front_spare_size(buffer_struc* buffer)
{
    return buffer->read_index;
}

void make_space(buffer_struc* buffer, int size)
{
    if (buffer_writeable_size(buffer) >= size) {
        return;
    }
    // no need alloc memory, just move it to get more free continous memory.
    if (buffer_front_spare_size(buffer) + buffer_writeable_size(buffer) >= size) {
        int i;
        int readable = buffer_readable_size(buffer);
        for (i = 0;i < readable;i++) {
            memcpy(buffer->data + i, buffer->data + buffer->read_index + i, 1);
        }
        buffer->read_index = 0;
        buffer->write_index = readable;
    }
    else {
        // expand the space to save the data.
        void* tmp = realloc(buffer->data, buffer->total_size + size);
        if (tmp == NULL) {
            return;
        }
        buffer->data = tmp;
        buffer->total_size += size;
    }
    return;
}

int buffer_append(buffer_struc* buffer, void* data, int size)
{
    if (data != NULL) {
        make_space(buffer, size);
        memcpy(buffer->data + buffer->write_index, data, size);
        buffer->write_index += size;
    }
}

int buffer_append_char(buffer_struc* buffer,char c)
{
    make_space(buffer,1);
    buffer->data[buffer->write_index++] = c;
    return 0;
}

int buffer_append_string(buffer_struc* buffer, char* data)
{
    if(data != NULL){
        buffer_append(buffer,data,strlen(data));
    }
}
int buffer_socket_read(buffer_struc* buffer, int fd)
{
    char tmp_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];
    int max_writable = buffer_writeable_size(buffer);
    vec[0].iov_base = buffer->data + buffer->write_index;
    vec[1].iov_len = max_writable;
    vec[1].iov_base = tmp_buffer;
    vec[1].iov_len = sizeof(tmp_buffer);
    int result = readv(fd, vec, 2);
    if (result < 0) {
        return -1;
    }
    else if (result <= max_writable) {
        buffer->write_index += result;
    }
    else {
        buffer->write_index = buffer->total_size;
        buffer_append(buffer, tmp_buffer, result - max_writable);
    }

    return result;
}
char buffer_read_char(buffer_struc* buffer)
{
    return buffer->data[buffer->read_index++];
}
char* buffer_find_CRLF(buffer_struc* buffer)
{
    char* crlf = memmem(buffer->data + buffer->read_index,
        buffer_readable_size(buffer), CRLF, 2);
    return crlf;
}