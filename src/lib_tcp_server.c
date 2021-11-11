#include <netdb.h>
#include <fcntl.h>
#include "lib_gutil.h"

socket_acceptor_struct *socket_acceptor_init(int port){
    int listenfd;
    int status;
    int yes = 1;
    struct addrinfo hints,*res,*rp;
    socket_acceptor_struct *acceptor = malloc(sizeof(socket_acceptor_struct));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((status = getaddrinfo(NULL, htons(port), &hints, &res) != 0)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
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
    acceptor->listen_fd = listenfd;
    acceptor->listen_port = port;
    return acceptor;
}