#ifndef LIB_TCP_SERVER_H
#define LIB_TCP_SERVER_H

#include "lib_gutil.h"
/*********************** socket acceptor ************************/
typedef struct socket_acceptor_struct {
    int listen_port;
    int listen_fd;
}socket_acceptor_struct;
socket_acceptor_struct* socket_acceptor_init(int port);

typedef struct tcp_connection_struc {

}tcp_connection_struc;


typedef int (*connection_completed_call_back_f)(tcp_connection_struc* tcp_connection);
typedef int (*message_call_back_f)(buffer_struc* buffer, tcp_connection_struc* tcp_connection);
typedef int (*write_completed_call_back_f)(tcp_connection_struc* tcp_connection);
typedef int (*connection_closed_call_back_f)(tcp_connection_struc* tcp_connection);

typedef struct tcp_server_struc {
    int port;
    int thread_num;
    void* data;
    event_loop_struc* event_loop;
    socket_acceptor_struct* acceptor;
    thread_pool_struc* thread_pool;
    connection_completed_call_back_f connection_completed_callback;
    message_call_back_f message_callback;
    write_completed_call_back_f write_completed_callback;
    connection_closed_call_back_f connection_closed_callback;
}tcp_server_struc;

tcp_server_struc* tcp_server_init(event_loop_struc* event_loop, socket_acceptor_struct* acceptor,
    connection_completed_call_back_f connection_callback,
    message_call_back_f message_callback,
    write_completed_call_back_f write_completed_callback,
    connection_closed_call_back_f connection_closed_callback,
    int thread_num);

void tcp_server_start(tcp_server_struc* tcp_server);

void tcp_server_set_data(tcp_server_struc* tcp_server, void* data);
#endif