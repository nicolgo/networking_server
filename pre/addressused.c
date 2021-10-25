#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>

#define PORT "12345"
#define BACKLOG 10

int main(int argc, char* argv[])
{
    int status;
    int listenfd;
    int cli_fd;
    int ret;
    char buf[1024];
    int yes = 1;
    struct addrinfo hints, * res, * rp;
    struct sockaddr_storage cli_addr;
    int cliaddr_size = sizeof(cli_addr);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, PORT, &hints, &res) != 0)) {
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
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, BACKLOG) != 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    if ((cli_fd = accept(listenfd, (struct sockaddr*)&cli_addr, &cliaddr_size)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        ret = read(cli_fd, buf, sizeof(buf) - 1);
        if (ret == 0) {
            perror("client closed");
            exit(EXIT_FAILURE);
        }
        else if (ret < 0) {
            perror("error read");
            exit(EXIT_FAILURE);
        }
        buf[ret] = 0x00;
        printf("received %d bytes: %s\n", ret, buf);
    }

    exit(EXIT_SUCCESS);
}