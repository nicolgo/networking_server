#include "lib_gutil.h"

#define MAX_LINE 1024
#define FD_INIT_SIZE 128

struct Buffer {
    int connect_fd;
    char buffer[MAX_LINE];
    size_t write_index;//the current writeable position
    size_t read_index;//the current readable position
    int readable;
};

struct Buffer* alloc_Buffer() {
    struct Buffer* buffer = malloc(sizeof(struct Buffer));
    if (!buffer)
        return NULL;
    buffer->connect_fd = 0;
    buffer->write_index = buffer->read_index = buffer->readable = 0;
    return buffer;
}

void free_Buffer(struct Buffer* buffer) {
    free(buffer);
}

char rot13_char(char c)
{
    if ((c >= 'a' && c <= 'm') || (c >= 'A') && (c <= 'M')) {
        return c + 13;
    }
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) {
        return c - 13;
    }
    else {
        return c;
    }
}
// server read data from client, then write it to buffer,
// finally mark it ready to be read.
int on_socket_read(int fd, struct Buffer* buffer)
{
    char buf[1024];
    int i;
    ssize_t result;
    while (1) {
        result = recv(fd, buf, sizeof(buf), 0);
        if (result < 0) {
            break;
        }

        for (i = 0;i < result;i++) {
            if (buffer->write_index < sizeof(buffer->buffer)) {
                buffer->buffer[buffer->write_index++] = rot13_char(buf[i]);
            }
            if (buf[i] == '\n') {
                buffer->readable = 1;//buffer is readable
            }
        }
    }
    if (result == 0) {
        return 1;//EOF
    }
    else if (result < 0) {
        if (errno == EAGAIN) {
            return 0;
        }
        return -1;
    }
    return 0;
}
// server send data to client, finally mark cannot readable.
int on_socket_write(int fd, struct Buffer* buffer) {
    while (buffer->read_index < buffer->write_index) {
        ssize_t result = send(fd, buffer->buffer + buffer->read_index, buffer->write_index - buffer->read_index, 0);
        if (result < 0) {
            if (errno == EAGAIN) {
                return 0;
            }
            return -1;
        }
        buffer->read_index += result;
    }

    if (buffer->read_index == buffer->write_index) {
        buffer->read_index = buffer->write_index = 0;
    }
    //the data in buffer is send out.
    buffer->readable = 0;

    return 0;
}

int main(int argc, char* argv[])
{
    int listen_fd;
    int i;
    int max_fd;
    fd_set readset, writeset, exset;

    struct Buffer* buffer[FD_INIT_SIZE];
    for (i = 0;i < FD_INIT_SIZE;i++) {
        buffer[i] = alloc_Buffer();
    }

    listen_fd = tcp_nonblocking_server(NULL,"12345");
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);

    while (1) {
        max_fd = listen_fd;

        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        FD_SET(listen_fd, &readset);
        for (i = 0;i < FD_INIT_SIZE;i++) {
            if (buffer[i]->connect_fd > 0) {
                max_fd = buffer[i]->connect_fd;
            }
            FD_SET(buffer[i]->connect_fd, &readset);
            if (buffer[i]->readable) {
                FD_SET(buffer[i]->connect_fd, &writeset);
            }
        }

        if (select(max_fd + 1, &readset, &writeset, &exset, NULL) < 0) {
            report_error("select error");
        }

        if (FD_ISSET(listen_fd, &readset)) {
            struct sockaddr_storage cli_addr;
            socklen_t addr_len = sizeof(cli_addr);
            int new_fd;
            printf("listening socket readable\n");
            sleep(5);//very important.RST will make block accept.
            new_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &addr_len);
            if (new_fd < 0) {
                report_error("fail to accept");
            }
            else if (new_fd > FD_INIT_SIZE) {
                close(new_fd);
                report_error("too many connections");
            }
            else {
                fcntl(new_fd, F_SETFL, O_NONBLOCK);
                if (buffer[new_fd]->connect_fd == 0) {
                    buffer[new_fd]->connect_fd = new_fd;
                }
                else {
                    report_error("too many connections");
                }
            }
        }

        for (i = 0;i < max_fd + 1;++i) {
            int r = 0;
            if (i == listen_fd) {
                continue;
            }
            if (FD_ISSET(i, &readset)) {
                r = on_socket_read(i, buffer[i]);
            }
            if (r == 0 && FD_ISSET(i, &writeset)) {
                r = on_socket_write(i, buffer[i]);
            }
            if (r) {
                buffer[i]->connect_fd = 0;
                close(i);
            }
        }

    }

}