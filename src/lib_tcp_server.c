#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include "buffer.h"
#include "lib_gutil.h"
#include "channel.h"

socket_acceptor_struct* socket_acceptor_init(int port) {
    int listenfd;
    int status;
    int yes = 1;
    char port_str[6];
    struct addrinfo hints, * res, * rp;
    socket_acceptor_struct* acceptor = malloc(sizeof(socket_acceptor_struct));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    snprintf(port_str, sizeof(port_str), "%d", port);
    if ((status = getaddrinfo(NULL, port_str, &hints, &res) != 0)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        fcntl(listenfd, F_SETFL, O_NONBLOCK);
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        //fprintf(stderr,"%s\n",strerror(errno));
        close(listenfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, 10) != 0) {
        perror("failed to listen");
        exit(EXIT_FAILURE);
    }
    acceptor->listen_fd = listenfd;
    acceptor->listen_port = port;
    return acceptor;
}


/******************** tcp connection **************************/

int handle_connection_closed(tcp_connection_struc* tcp_connection)
{
    event_loop_struc *event_loop = tcp_connection->event_loop;
    channel_struc *channel = tcp_connection->channel;
    event_loop_remove_channel_event(event_loop,channel->fd,channel);
    if(tcp_connection->on_connection_closed != NULL){
        tcp_connection->on_connection_closed(tcp_connection);
    }
    return 0;
}

int handle_read(void* data)
{
    tcp_connection_struc* tcp_connection = (tcp_connection_struc*)data;
    buffer_struc* input_buffer = tcp_connection->input_buffer;
    channel_struc* channel = tcp_connection->channel;

    if (buffer_socket_read(input_buffer, channel->fd) > 0) {
        if (tcp_connection->on_message != NULL) {
            tcp_connection->on_message(input_buffer, tcp_connection);
        }
    }
    else {
        handle_connection_closed(tcp_connection);
    }

    return 0;
}

int handle_write(void* data)
{
    tcp_connection_struc* tcp_connection = (tcp_connection_struc*)data;
    event_loop_struc* event_loop = tcp_connection->event_loop;
    buffer_struc* output_buffer = tcp_connection->out_buffer;
    channel_struc* channel = tcp_connection->channel;
    ssize_t n_writed = write(channel->fd, output_buffer->data + output_buffer->read_index,
        buffer_readable_size(output_buffer));
    if (n_writed > 0) {
        output_buffer->read_index += n_writed;
        if (buffer_readable_size(output_buffer) == 0) {
            channel_write_event_disable(channel);
        }
        if (tcp_connection->on_write_completed != NULL) {
            tcp_connection->on_write_completed(tcp_connection);
        }
    }
    else {
        net_msgx("tcp connection write %s", tcp_connection->name);
    }
    return 0;
}

tcp_connection_struc* tcp_connection_init(int connected_fd, event_loop_struc* event_loop,
    connection_completed_callback_f on_connection_completed,
    connection_closed_callback_f on_connection_closed,
    message_callback_f on_message,
    write_completed_callback_f on_write_completed)
{
    tcp_connection_struc* tcp_connection = malloc(sizeof(tcp_connection_struc));
    tcp_connection->on_write_completed = on_write_completed;
    tcp_connection->on_message = on_message;
    tcp_connection->on_connection_completed = on_connection_completed;
    tcp_connection->on_connection_closed = on_connection_closed;
    tcp_connection->event_loop = event_loop;
    tcp_connection->input_buffer = buffer_new();
    tcp_connection->out_buffer = buffer_new();

    char buf[16] = { 0 };
    sprintf(buf, "connection-%d\0", connected_fd);
    tcp_connection->name = buf;

    channel_struc* channel = channel_init(connected_fd, EVENT_READ,
        handle_read, handle_write, tcp_connection);
    tcp_connection->channel = channel;

    if (tcp_connection->on_connection_completed != NULL) {
        tcp_connection->on_connection_completed(tcp_connection);
    }

    event_loop_add_channel_event(tcp_connection->event_loop, connected_fd,
        tcp_connection->channel);

    return tcp_connection;
}

int tcp_connection_send_data(tcp_connection_struc* tcp_connection, void* data, int size)
{
    size_t n_writed = 0;
    size_t n_left = size;
    int fault = 0;
    channel_struc* channel = tcp_connection->channel;
    buffer_struc* output_buffer = tcp_connection->out_buffer;

    if (!channel_write_event_is_enabled(channel)
        && buffer_readable_size(output_buffer) == 0) {
        n_writed = write(channel->fd, data, size);
        if (n_writed >= 0) {
            n_left = n_left - n_writed;
        }
        else {
            n_writed = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }

    if (!fault && n_left > 0) {
        buffer_append(output_buffer, data + n_writed, n_left);
        if (!channel_write_event_is_enabled(channel)) {
            channel_write_event_enable(channel);
        }
    }

    return n_writed;
}
int tcp_connection_send_buffer(tcp_connection_struc* tcp_connection, buffer_struc* buffer)
{
    int size = buffer_readable_size(buffer);
    int result = tcp_connection_send_data(tcp_connection, buffer->data + buffer->read_index, size);
    buffer->read_index += size;
    return result;
}
void tcp_connection_shutdown(tcp_connection_struc* tcp_connection)
{
    if (shutdown(tcp_connection->channel->fd, SHUT_WR) < 0) {
        net_msgx("failed to shutdown tcp connect on socket=%d", tcp_connection->channel->fd);
    }
}


/************************* TCP server *************************************/

tcp_server_struc* tcp_server_init(event_loop_struc* event_loop, socket_acceptor_struct* acceptor,
    connection_completed_callback_f connection_callback,
    message_callback_f message_callback,
    write_completed_callback_f write_completed_callback,
    connection_closed_callback_f connection_closed_callback,
    int thread_num)
{
    tcp_server_struc* tcp_server = malloc(sizeof(tcp_server_struc));
    tcp_server->event_loop = event_loop;
    tcp_server->acceptor = acceptor;

    tcp_server->connection_completed_callback = connection_callback;
    tcp_server->message_callback = message_callback;
    tcp_server->write_completed_callback = write_completed_callback;
    tcp_server->connection_closed_callback = connection_closed_callback;

    tcp_server->thread_num = thread_num;
    tcp_server->thread_pool = thread_pool_init(event_loop, thread_num);
    tcp_server->data = NULL;

    return tcp_server;
}

void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int handle_connection_established(void* data)
{
    tcp_server_struc* tcp_server = (tcp_server_struc*)data;
    socket_acceptor_struct* acceptor = tcp_server->acceptor;
    int listen_fd = acceptor->listen_fd;
    struct sockaddr_storage cli_addr;
    socklen_t addr_len = sizeof(cli_addr);
    int connected_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &addr_len);
    set_nonblocking(connected_fd);

    net_msgx("new connection established on socket %d", connected_fd);

    event_loop_struc* event_loop = thread_pool_get_loop(tcp_server->thread_pool);


    return 0;
}

void tcp_server_start(tcp_server_struc* tcp_server)
{
    socket_acceptor_struct* acceptor = tcp_server->acceptor;
    event_loop_struc* event_loop = tcp_server->event_loop;
    //start thread pool
    thread_pool_start(tcp_server->thread_pool);
    // main thread with acceptor
    channel_struc* channel = channel_init(acceptor->listen_fd, EVENT_READ,
        handle_connection_established, NULL, tcp_server);
    event_loop_add_channel_event(event_loop, channel->fd, channel);

    return;
}

void tcp_server_set_data(tcp_server_struc* tcp_server, void* data)
{
    if (data != NULL) {
        tcp_server->data = data;
    }
}