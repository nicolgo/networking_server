#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define SERVPORT "43211"
#define BACKLOG 10
#define MAXLINE 4096

static int count;

static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char* argv[])
{
    int status;
    int sfd;
    struct addrinfo hints, * res, * rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, SERVPORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    for (rp = res;rp != NULL;rp = rp->ai_next) {
        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sfd);
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    if (listen(sfd, BACKLOG) == -1) {
        fprintf(stderr, "Could not listen\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_DFL);

    int connfd;
    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof client_addr;

    if ((connfd = accept(sfd, (struct sockaddr*)&client_addr, &sin_size)) < 0) {
        fprintf(stderr, "bind failed.\n");
    }

    char message[MAXLINE];
    count = 0;

    for (;;) {
        int n = read(connfd, message, MAXLINE);
        if (n < 0) {
            fprintf(stderr, "error read\n");
        }
        else if (n == 0) {
            fprintf(stderr, "client closed\n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;

        char send_line[MAXLINE + 10];
        sprintf(send_line, "Hi, %s", message);

        sleep(5);

        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %d \n", write_nc);
        if (write_nc < 0) {
            fprintf(stderr, "error write");
        }
    }
}