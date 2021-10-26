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
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            perror("connect failed!\n");
            close(sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        perror("failed to connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}