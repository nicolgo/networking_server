#include <netdb.h>
#include <fcntl.h>
#include "lib_gutil.h"

socket_acceptor_struct* socket_acceptor_init(int port) {
    int listenfd;
    int status;
    int yes = 1;
    char port_str[5];
    struct addrinfo hints, * res, * rp;
    socket_acceptor_struct* acceptor = malloc(sizeof(socket_acceptor_struct));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    itoa(port, port_str, 10);
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


tcp_server_struc* tcp_server_init(event_loop_struc* event_loop, socket_acceptor_struct* acceptor,
    connection_completed_call_back_f connection_callback,
    message_call_back_f message_callback,
    write_completed_call_back_f write_completed_callback,
    connection_closed_call_back_f connection_closed_callback,
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
    tcp_server->thread_pool = thread_pool_init(event_loop,thread_num);
    tcp_server->data = NULL;

    return tcp_server;
}

void tcp_server_start(tcp_server_struc* tcp_server)
{

}

void tcp_server_set_data(tcp_server_struc* tcp_server, void* data)
{
    if(data != NULL){
        tcp_server->data = data;
    }
}