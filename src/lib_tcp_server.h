#ifndef LIB_TCP_SERVER_H
#define LIB_TCP_SERVER_H

#include "thread_pool.h"
#include "lib_gutil.h"


/*********************** socket acceptor ************************/
typedef struct socket_acceptor_struct {
    int listen_port;
    int listen_fd;
}socket_acceptor_struct;
socket_acceptor_struct* socket_acceptor_init(int port);

/********************** tcp connection *************************/
typedef int (*connection_completed_callback_f)(struct tcp_connection_struc* tcp_connection);
typedef int (*message_callback_f)(buffer_struc* buffer, struct tcp_connection_struc* tcp_connection);
typedef int (*write_completed_callback_f)(struct tcp_connection_struc* tcp_connection);
typedef int (*connection_closed_callback_f)(struct tcp_connection_struc* tcp_connection);

typedef struct tcp_connection_struc {
    char* name;
    event_loop_struc* event_loop;
    channel_struc* channel;
    buffer_struc* input_buffer;
    buffer_struc* out_buffer;
    // the parameter of callback function 
    void* data;
    void* request;
    void* response;
    // callback functions
    connection_completed_callback_f on_connection_completed;
    message_callback_f on_message;
    write_completed_callback_f on_write_completed;
    connection_closed_callback_f on_connection_closed;
}tcp_connection_struc;

tcp_connection_struc* tcp_connection_init(int connected_fd, event_loop_struc* event_loop,
    connection_completed_callback_f on_connection_completed,
    connection_closed_callback_f on_connection_closed,
    message_callback_f on_message,
    write_completed_callback_f on_write_completed);

int tcp_connection_send_data(tcp_connection_struc* tcp_connection, void* data, int size);
int tcp_connection_send_buffer(tcp_connection_struc* tcp_connection, buffer_struc* buffer);
void tcp_connection_shutdown(tcp_connection_struc* tcp_connection);

/*********************** tcp server *****************************/
typedef struct tcp_server_struc {
    int port;
    int thread_num;
    void* data;
    event_loop_struc* event_loop;
    socket_acceptor_struct* acceptor;
    struct thread_pool_struc* thread_pool;
    connection_completed_callback_f connection_completed_callback;
    message_callback_f message_callback;
    write_completed_callback_f write_completed_callback;
    connection_closed_callback_f connection_closed_callback;
}tcp_server_struc;

tcp_server_struc* tcp_server_init(event_loop_struc* event_loop, socket_acceptor_struct* acceptor,
    connection_completed_callback_f connection_callback,
    message_callback_f message_callback,
    write_completed_callback_f write_completed_callback,
    connection_closed_callback_f connection_closed_callback,
    int thread_num);

void tcp_server_start(tcp_server_struc* tcp_server);

void tcp_server_set_data(tcp_server_struc* tcp_server, void* data);

void set_nonblocking(int fd);

#endif