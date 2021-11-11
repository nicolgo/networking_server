#ifndef LIB_TCP_SERVER_H
#define LIB_TCP_SERVER_H

#include "lib_gutil.h"

typedef struct socket_acceptor_struct{
    int listen_port;
    int listen_fd;
}socket_acceptor_struct;

socket_acceptor_struct *socket_acceptor_init(int port);

typedef struct tcp_connection_struc{

}tcp_connection_struc;

typedef struct tcp_server_struc{
    int port;
    int threadNum;
    void *data;
    event_loop_struc *event_loop;
    socket_acceptor_struct *acceptor;
}tcp_server_struc;
#endif