#include "lib_gutil.h"

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