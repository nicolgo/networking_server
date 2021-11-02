#include "lib_gutil.h"

int tcp_client(char* address, char* port)
{
    int sockfd;
    int status;
    struct addrinfo hints, * res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (address == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    status = getaddrinfo(address, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            perror("create socket failed!\n");
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int tcp_nonblocking_server(char* host_or_ip, char* ser_port)
{
    int listenfd;
    int status;
    int yes = 1;
    struct addrinfo hints,*res,*rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if(host_or_ip == NULL){
        hints.ai_flags = AI_PASSIVE;
    }

    if ((status = getaddrinfo(host_or_ip, ser_port, &hints, &res) != 0)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        // set non blocking IO
        fcntl(listenfd,F_SETFL,O_NONBLOCK);
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
    return listenfd;
}

int tcp_listen_fd(char* host_or_ip, char* serv_port)
{
    int listenfd;
    int status;
    int yes = 1;
    struct addrinfo hints,*res,*rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if(host_or_ip == NULL){
        hints.ai_flags = AI_PASSIVE;
    }

    if ((status = getaddrinfo(host_or_ip, serv_port, &hints, &res) != 0)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        
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
    return listenfd;
}

void report_error(char* err_msg)
{
    perror(err_msg);
    exit(EXIT_FAILURE);
}

char lib_rot13_char(char c)
{
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M')) {
        return c + 13;
    }
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) {
        return c - 13;
    }
    else {
        return c;
    }
}

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_accept_fd(int listen_fd)
{
    struct sockaddr_storage cli_addr;
    socklen_t addr_len = sizeof(cli_addr);
    int fd;

    fd = accept(listen_fd,(struct sockaddr*)&cli_addr,&addr_len);

    return fd;
}